/**
* PC Sampling analysis based on data read from rocprofv3
 * Parses the assembly file to extract unmangled kernel names
 *
 * @author Lukas Eckert
 */

#ifndef GPUSCOUT_PARSER_PCSAMPLING_HPP
#define GPUSCOUT_PARSER_PCSAMPLING_HPP


#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include "../utilities/json.hpp"
#include "amd_helper.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;


// Used for matching the individual pc sampling line
std::regex pc_pattern()
{
    /*
     *   21.1 PC Sampling
     *   ╒════╤═════════════════╤══════════════════════════════════════════════════════════════╤══════════╤═════════╤════════════════╤═════════════════╤═════════════════════════════════════════════════════════════════╕
     *   │    │ source_line     │ instruction                                                  │   offset │   count │   count_issued │   count_stalled │ stall_reason                                                    │
     *   ╞════╪═════════════════╪══════════════════════════════════════════════════════════════╪══════════╪═════════╪════════════════╪═════════════════╪═════════════════════════════════════════════════════════════════╡
     *   │ 0  │ ../fp8.cpp:63   │ s_nop 1                                                      │ 0x1658   │      38 │              0 │              38 │ [('NO_INSTRUCTION_AVAILABLE', 29), ('INTERNAL_INSTRUCTION', 9)] │
     *   ├────┼─────────────────┼──────────────────────────────────────────────────────────────┼──────────┼─────────┼────────────────┼─────────────────┼─────────────────────────────────────────────────────────────────┤
     *   │ 0  │ ../fp8.cpp:63   │ v_mfma_f32_32x32x16_fp8_fp8 v[2:17], v[0:1], v[0:1], v[2:17] │ 0x165c   │      38 │              0 │              38 │ [('ARBITER_NOT_WIN', 128), ('NO_INSTRUCTION_AVAILABLE', 11)]    │
     *   ╘════╧═════════════════╧══════════════════════════════════════════════════════════════╧══════════╧═════════╧════════════════╧═════════════════╧═════════════════════════════════════════════════════════════════╛
    */
    return std::regex(
        "^│"                                        // left border
        "\\s*"                                      // whitespaces after border
        "\\d*"                                      // pc stall index
        "\\s*│"                                     // whitespaces after index
        "\\s*([^│\\s:]*):([^│\\s]*)\\s*│"           // source_line (group 1 file, group 2 line number)
        "\\s*([^│]*[^\\s│]+)\\s*│"                  // instruction without whitespace at end group 3
        "\\s*([^│\\s]*)\\s*│"                       // offset group 4
        "\\s*([^│\\s]*)\\s*│"                       // count group 5
        "\\s*([^│\\s]*)\\s*│"                       // count_issued group 6
        "\\s*([^│\\s]*)\\s*│"                       // count_stalled group 7
        "\\s*(\\[[^│]*\\])\\s*│"                    // stall_reason group 8
    );
}


/// @brief Kind of bottleneck analysis performed
enum analysis_kind
{
    ALL,
    REGISTER_SPILLING,
    VECTORIZED_LOAD,
    ATOMICS_GLOBAL,
    RESTRICT_USE,
    WARP_DIVERGENCE,
    TEXTURE_USE,
    SHARED_USE,
    DATATYPE_CONVERSION,
};

struct pc_issue_samples
{
    std::string pc_offset;
    int line_number;
    std::string assembly_instruction;
    std::vector<std::pair<std::string, int>> stall_name_count_pair;
};



/// @brief Get the warp stall reasons and their corresponding stall values with the corresponding pcOffset
/// @param  directory containing pc stall information  files
/// @param  object file for matching mangled kernel names
/// @return Vector of stall reasons and stall values of the relevant assembly instructions for each kernel
std::unordered_map<std::string, std::vector<pc_issue_samples>> get_warp_stalls(const std::string &dir, const std::string &assembly_filename)
{
    // TODO uncomment the line below and remove the following two lines. Those only act for testing the code right now
    //std::unordered_map<std::string, std::string> kernel_names_table = build_kernel_names_table(assembly_filename);
    std::unordered_map<std::string, std::string> kernel_names_table;
    kernel_names_table.insert({"vectorKernel(float*, float*)","_Z12vectorKernelPfS_"});

    std::unordered_map<std::string, std::vector<pc_issue_samples>> pc_samples_map;

    // Iterate over files in the given pc samples folder
    for (const auto& entry : fs::directory_iterator(dir)) {
        // check if the current entry is a file
        if (fs::is_regular_file(entry.status())) {
            std::string filename = entry.path().filename().string();

            // regular expression to match files ending with _<kernel_name>_pc_samples.txt
            std::regex file_pattern(".*_([\\w]+)_pc_samples\\.txt$");
            std::smatch file_match;
            if (std::regex_match(filename, file_match, file_pattern)) {
                std::vector<pc_issue_samples> pc_obj; // zero initialized to prevent undefined behaviour
                std::string krn_name = "ERROR"; // No error catching needed because metrics wont be added when no krn_name was set
                bool krn_name_set = false;
                std::ifstream file(entry.path());

                if (file.is_open()) {
                    std::string line;

                    // Iterate through file
                    while (std::getline(file, line)) {
                        std::smatch line_match;

                        // Kernel name parsing - only needed once per metrics file
                        if (krn_name_set == false) {
                            if (std::regex_match(line, line_match, kernel_name_pattern())) {
                                krn_name = kernel_names_table[line_match[1].str()];
                                krn_name_set = true;
                            }
                            else {
                                continue;
                            }
                        }

                        // PC Sample Parsing
                        else {
                            // Found entry in PC Sampling section
                            if(std::regex_search(line, line_match, pc_pattern())) {
                                pc_issue_samples samp_obj;
                                std::vector<std::pair<std::string, int>> stall_name_count_pair;

                                // Iterate over reasons
                                std::string reasons = line_match[8].str();
                                // [('NO_INSTRUCTION_AVAILABLE', 29), ('INTERNAL_INSTRUCTION', 9)]
                                std::regex reason_pair_regex(R"('([^\s]*)', ([0-9]*))");
                                auto words_begin = std::sregex_iterator(reasons.begin(), reasons.end(), reason_pair_regex);
                                auto words_end = std::sregex_iterator();

                                for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                                    std::smatch reason_match = *i;
                                    std::string reason = reason_match[1].str();
                                    int reason_count = std::stoi(reason_match[2].str());
                                    stall_name_count_pair.push_back(std::make_pair(reason, reason_count));
                                }

                                // Create pc sample object
                                samp_obj.assembly_instruction = line_match[3].str();
                                samp_obj.line_number = std::stoi(line_match[2].str());
                                samp_obj.pc_offset = line_match[4].str();
                                samp_obj.stall_name_count_pair = stall_name_count_pair;

                                pc_obj.push_back(samp_obj);
                            }
                        }
                    }
                }
                else {
                    std::cerr << "Unable to open file " << entry.path().string() << std::endl;
                }

                pc_samples_map[krn_name] = pc_obj;
            }
        }
    }

    return pc_samples_map;
}

#endif //GPUSCOUT_PARSER_PCSAMPLING_HPP