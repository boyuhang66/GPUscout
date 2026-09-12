#include "parser_amdgcn_register_spilling.hpp"
#include "parser_liveregisters.hpp"
#include "parser_metrics.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>

using json = nlohmann::json;

/*!
 * Build the part of the final register-spilling result that can already be determined from static AMDGCN assembly analysis.
 * The JSON structure intentionally matches the existing final output structure. 
 * Dynamic information such as register pressure is added later.
 */
json build_static_register_spilling_result(const std::unordered_map<std::string, std::vector<mem>>& mem_map)
{
    json result;

    for (const auto& [krn_name, mem_vec] : mem_map)
    {
        json krn_result = {
            {"occurrences", json::array()}
        };

        // TODO: check if this is happening
        if (krn_name == "")
        {
            break;
        }

        for (const auto& mem_obj : mem_vec)
        {
            if (mem_obj.type != WRITE && mem_obj.type != STORE)
            {
                continue;
            }

            json line_result = {
                {"file_name", mem_obj.loc.file_name},
                {"line_number", mem_obj.loc.line_num},
                {"pc_offset", mem_obj.PC_offset},
                {"instruction", mem_obj.name},
                {"register", mem_obj.reg_num},
                {"operation", mem_obj.type}
            };

            if (mem_obj.successor == true)
            {
                line_result["previous_compute_instruction"] = {
                    {"instruction", mem_obj.fst.name},
                    {"file_name", mem_obj.fst.loc.file_name},
                    {"line_number", mem_obj.fst.loc.line_num}/*,
                    {"pc_offset", 0} // TODO pc_offset*/
                };
            }
             krn_result["occurrences"].push_back(line_result);
        }

        result[krn_name] = krn_result;
    }

    return result;
}

/*!
 * Return true if at least one register-spilling occurrence was found during static analysis.
 */
bool has_register_spilling_candidate(const json& result)
{
    for (const auto& [krn_name, krn_result] : result.items())
    {
        if (!krn_result["occurrences"].empty())
        {
            return true;
        }
    }

    return false;
}

/*!
 * Enrich the previously generated static result with dynamic information and print the complete analysis result to the terminal.
 */
json analysis_register_spilling (
    json result,
    std::unordered_map<std::string, mtc> mtc_map,
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map)
{
    for (auto& [krn_name, krn_result] : result.items())
    { 
        auto& occurrences = krn_result["occurrences"];

        if (occurrences.empty())
        {
            std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no register spilling detected in kernel " << krn_name << std::endl;
            continue;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : register spilling" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        
        
        for (auto& line_result : occurrences)
        {
            std::cout << std::endl; 
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== register spilling detected in file " << line_result["file_name"].get<std::string>() << " at line "
                          << line_result["line_number"].get<int>() << std::endl;
                std::cout << "==== instruction: " << line_result["instruction"].get<std::string>() << std::endl;
                std::cout << "==== vdata register: " << line_result["register"].get<std::string>() << std::endl;

                if (line_result.contains("previous_compute_instruction"))
                {
                    const auto& previous = line_result["previous_compute_instruction"];
                    std::cout << "==== the succeeding compute instruction using register " << line_result["register"].get<std::string>()
                              <<std::endl;
                    std::cout << "     after spilling was " << previous["instruction"].get<std::string>() << " in file "
                              << previous["file_name"].get<std::string>() << " at line number " << previous["line_number"].get<int>()
                              << std::endl;
                }

                // --------- Register Pressure ------------------
                // search for a match between the PC offset of the current local memory instruction and the PC offset of
                // an entry in the live register map
                const std::string pc_offset = line_result["pc_offset"].get<std::string>();
                auto reg_search_it = std::find_if(
                    live_register_map[krn_name].begin(),
                    live_register_map[krn_name].end(),
                    [&](const auto &i) { return pc_offset == i.pcOffset; }
                    );

                if (reg_search_it != live_register_map[krn_name].end()) {
                    std::cout << "==== INFO :: Current VGPRs for the AMDGCN ISA instruction: " << reg_search_it->vgp_reg << std::endl;
                    line_result["used_register_count"] = reg_search_it->vgp_reg;

                    if (reg_search_it->change_vgpr_from_last > 0) {
                        std::cout << "Increased VGPR pressure with " << std::abs(reg_search_it->change_vgpr_from_last) << " more registers compared to last AMGGCN ISA instruction" << std::endl;
                        line_result["register_pressure_increase"] = std::abs(reg_search_it->change_vgpr_from_last);
                    } else {
                        line_result["register_pressure_increase"] = 0;
                    }
                }

                // TODO PC stall
        }

        // Metrics
        auto mtc_obj = mtc_map[krn_name];

        auto approx_percent = mtc_obj.ID_17_3_1 ? mtc_obj.ID_15_2_5/*mtc_obj.ID_15_1_9*/ * mtc_obj.ID_16_3_5 / mtc_obj.ID_17_3_1 : 0.0;

        std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;
        std::cout << "==== total number of spill/stack memory instructions executed on all compute units"
                  << " on the accelerator" << std::endl;
        std::cout << "     " << mtc_obj.ID_15_2_5/*mtc_obj.ID_15_1_9*/ << std::endl;
        std::cout << "==== number of cycles the address processing unit spent working on spill/stack instructions"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_15_3_0/*mtc_obj.ID_15_1_13*/ << std::endl;
        std::cout << "==== APPROXIMATE percentage of total L2 cache requests due to private memory" << std::endl;
        std::cout << "==== if the percentage is high, the memory traffic between the CUs and L2 is mostly due to"
                  << " private" << std::endl;
        std::cout << "     st memory (need to contain register spills)" << std::endl;
        std::cout << "     " << approx_percent << std::endl;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    const auto static_result_file = static_result_path(assembly, "register_spilling");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the static part of the final analysis result.
     *  3. If a candidate exists, preserve the result for the later full-analysis stage.
     *
     *  exit 0 -> register spilling detected
     *  exit 1 -> no register spilling detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto mem_map = parser_register_spilling(assembly);

        json result = build_static_register_spilling_result(mem_map);
        if (!has_register_spilling_candidate(result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, result))
        {
            std::cerr << "ERROR: Could not save static register spilling result to "
                      << static_result_file << std::endl;
            return 2;
        }

        return 0;
    }
        
    /*! Full analysis mode:
     *  exit 0 -> successful analysis
     *  exit 2 -> invalid arguments
     */
    if (argc < 6)
    {
        std::cerr << "ERROR: Invalid arguments for register spilling analysis."
                  << std::endl;
        return 2;
    }

    json result;
    /*
     * Automatic mode: reuse the static result generated during detection.
     * Manual mode: no static result exists, therefore perform the original assembly parsing here.
     */
    if (!load_static_result(static_result_file, result))
    {
        auto mem_map = parser_register_spilling(assembly);
        result = build_static_register_spilling_result(mem_map);
    }


    //TODO PC stalls

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    // live registers
    std::string livereg_dir = argv[3];
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map = live_registers_analysis(livereg_dir, assembly);

    bool save_as_json = std::strcmp(argv[4], "true") == 0;
    std::string json_out_dir = argv[5];

    result = analysis_register_spilling(result, mtc_map, live_register_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/register_spilling.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
