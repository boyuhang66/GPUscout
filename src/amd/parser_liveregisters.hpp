/**
* For each instruction, currently used (or live) registers saved
 * Additonally computes if there is an increase in register usage compared to last instruction
 * Lower the number of registers used, more is the occupancy and more threads can be run per block
 *
 * @author Lukas Eckert
 */

#ifndef PARSER_LIVEREGISTERS_HPP
#define PARSER_LIVEREGISTERS_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

inline std::regex regex_live_register_start() {
    return std::regex(
        "^"                                       // regex must match from the beginning of the string
        "-*"                                      // matches the ----------- line/beginning of the analysis
    );
}

inline std::regex regex_live_register() {
    /*
     * Line | Rn  |                                                              | Instruction
    --------------------------------------------------------------------------------
          1 |   1 | :                                                            | s_load_dword s6, s[4:5], 0x1c
    */
    return std::regex(
        "^"                                       // regex must match from the beginning of the string
        "\\s*"                                    // matches any leading whitespace
        "(\\d*)"                                  // matches the line number
        "\\s\\|\\s*"                              // matches whitespace followed | and the leading whitespaces
        "(\\d*)"                                  // matches the register number
        "\\s\\|\\s*"                              // matches whitespace followed | and the leading whitespaces
        "([:^vx\\s]*)"                            // matches the register status part
        "\\|\\s*"                                 // matches the | and the leading whitespaces
        "([^\\s]+)"                               // matches the instruction or label
        "\\s"                                     // matches the following whitespace
        "([^\\s]+)"                               // matches the first operand or instruction if label was present
        ".*"                                      // matches anything following
    );
}

inline std::regex regex_assembly_instruction() {
    /*
                s_load_dword s6, s[4:5], 0x1c                              // 000000001E00: C0020182 0000001C
    */
    return std::regex(
        "^"                                       // regex must match from the beginning of the string
        "\\s+"                                    // matches any leading whitespace (at least one)
        "([^\\s]+)"                               // matches the instruction
        "(.*?)//"                                 // matches any following until // is encountered
        "\\s*"                                    // matches the following whitespaces
        "(.*?):"                                 // matches any following until : is encountered
        ".*"                                      // matches anything following
    );
}


/// @brief Each instruction contains number of currently active registers, corresponding to the pcOffset of the instruction and line inside of the kernel
struct live_registers
{
    int line;
    std::string pcOffset;
    std::string instruction;
    int vgp_reg;
    int change_reg_from_last; // change in number of registers compared to the last SASS instruction
};

/// @brief Extracts the kernel name of RGA generated live register analysis files
std::string extract_kernel_name(const std::string& filename) {
    // Example: gfx90a__Z12vectorKernelPfS__reg-spill-vec-vgpr.txt
    // <llvm target name>__Z<length of function name><function name>...
    auto pos = filename.find("_Z");
    if (pos == std::string::npos) {
        return {};
    }
    pos += 2; // skipping _Z

    size_t len = 0;
    while (pos < filename.size() && std::isdigit(filename[pos])) {
        len = len * 10 + (filename[pos] - '0');
        ++pos;
    }

    if (pos + len > filename.size()) {
        return {}; // malformed symbol
    }

    return filename.substr(pos, len);
}


/// @brief For every kernel, stores a vector of live registers
/// @param path to the metrics directory
/// @param filename assembly file
/// @return mapping of each kernel with a vector of live registers
std::unordered_map<std::string, std::vector<live_registers> > live_registers_analysis(const std::string &path, const std::string &assembly_filename) {
    std::vector<std::filesystem::path> filenames;
    std::regex vgpr_regex(R"(.*-vgpr\.txt$)", std::regex::icase);

    //Find all live register analysis files
    for (const auto& file : fs::directory_iterator(path)) {
        if (file.is_regular_file()) {
            std::string filename = file.path().filename().string();
            if (std::regex_match(filename, vgpr_regex)) {
                filenames.push_back(file.path());
            }
        }
    }


    // map (key: kernel name | value: vector of active registers per instruction)
    std::unordered_map<std::string, std::vector<live_registers>> live_reg_map;
    std::vector<live_registers> reg_vec;
    std::string kernel_name;

    // Go through every kernel register pressure file
    for (auto filename : filenames) {
        std::fstream file(filename, std::ios::in);
        std::string line;

        kernel_name = extract_kernel_name(filename);
        int last_inst_register_count = 0;
        reg_vec.clear();

        if (file.is_open()) {
            // Find beginning of register analysis
            while (std::getline(file, line)) {
                std::smatch match;

                if (std::regex_search(line, match, regex_live_register_start())) {
                    break;
                }
            }

            // Extract register information from analysis
            while (std::getline(file, line)) {
                std::smatch match;

                if (std::regex_search(line, match, regex_live_register())) {
                    live_registers reg_obj;

                    // Instruction-slot can either be label or instruction
                    // Since there is no instruction with label inside of its name its fine to search like this
                    if (match[4].str().find("label") != std::string::npos) {
                        reg_obj.line = std::stoi(match[1].str());
                        reg_obj.pcOffset = "";
                        reg_obj.instruction = match[5].str();
                        reg_obj.vgp_reg = std::stoi(match[2].str());
                        reg_obj.change_reg_from_last = reg_obj.vgp_reg - last_inst_register_count;
                    }
                    else {
                        reg_obj.line = std::stoi(match[1].str());
                        reg_obj.pcOffset = "";
                        reg_obj.instruction = match[4].str();
                        reg_obj.vgp_reg = std::stoi(match[2].str());
                        reg_obj.change_reg_from_last = reg_obj.vgp_reg - last_inst_register_count;
                    }

                    last_inst_register_count = reg_obj.vgp_reg;
                    reg_vec.push_back(reg_obj);
                }
            }
            live_reg_map[kernel_name] = reg_vec;
        }
        else
            std::cout << "Could not open the file" << filename << std::endl;
    }

    // Assembly file part
    kernel_name = "";
    std::fstream as_file(assembly_filename, std::ios::in);
    std::string as_line; // stores the current line in the assembly file
    std::string as_instr; // stores the current instruction in the assembly file
    int instr_counter;
    if (as_file.is_open()) {
        while (std::getline(as_file, as_line)) {
            std::smatch match;

            // New kernel in assembly file
            if (std::regex_search(as_line, match, regex_krn_name))
            {
                instr_counter = 0;
                kernel_name = match[1].str();
            }

            if (!kernel_name.empty() && std::regex_search(as_line, match, regex_assembly_instruction())) {
                as_instr = match[1].str();

                if (live_reg_map[kernel_name].size() > instr_counter) {
                    if (as_instr.find(live_reg_map[kernel_name][instr_counter].instruction) != std::string::npos) {
                        live_reg_map[kernel_name][instr_counter].pcOffset = match[3].str();

                        instr_counter++;
                    }
                }
            }
        }
    }
    else
        std::cout << "Could not open the file" << assembly_filename << std::endl;

    return live_reg_map;
}

#endif // PARSER_LIVEREGISTERS_HPP