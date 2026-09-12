#include "parser_amdgcn_restrict.hpp"
#include "parser_metrics.hpp"
#include "parser_liveregisters.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for the __restrict__ analysis.
 * All static information required by the final restrict.json output is already contained in "result"
 */
json build_static_restrict_result(const std::unordered_map<std::string, std::vector<reg>>& reg_map)
{
    json static_result = {
        {"result", json::object()}
    };

    for (const auto& [krn_name, reg_vec] : reg_map)
    {
        // TODO: check if this is happening
        if (krn_name.empty())
        {
            break;
        }

        json krn_result = {
            {"occurrences", json::array()}
        };

        for (const auto& reg_obj : reg_vec)
        {
            if (reg_obj.is_used)
            {
                continue;
            }

            krn_result["occurrences"].push_back({
                {"severity", "INFO"},
                {"pc_offset", reg_obj.PC_offset},
                {"file_name", reg_obj.loc.file_name},
                {"line_number", reg_obj.loc.line_num},
                {"register", reg_obj.reg_num}
            });
        }

        static_result["result"][krn_name] = krn_result;
    }

    return static_result;
}

/*!
 * Return true if at least one register could benefit from __restrict__.
 */
bool has_restrict_candidate(const json& static_result)
{
    for (const auto& [krn_name, krn_result] : static_result["result"].items())
    {
        if (!krn_result["occurrences"].empty())
        {
            return true;
        }
    }

    return false;
}

json analysis_restrict(
    json static_result,
    const std::unordered_map<std::string, mtc>& mtc_map,
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map)
{
    auto& result = static_result["result"];

    for (auto& [krn_name, krn_result] : result.items())
    {
	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : __restrict__" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

       auto& occurrences = krn_result["occurrences"];

        for (auto& occurrence : occurrences)
        {
            std::cout << std::endl; 
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== register " << occurrence["register"].get<std::string>() << ", first appearing used by global load in file "
                        << occurrence["file_name"].get<std::string>() << " at line " << occurrence["line_number"].get<int>() << " of your" << std::endl;
            std::cout << "     code, could benefit from using __restrict__" << std::endl;
    

            /* TODO Restrict analysis currently doesnt include pcOffset which is needed for matching register pressure
            // --------- Register Pressure ------------------
            // search for a match between the PC offset of the current local memory instruction and the PC offset of
            // an entry in the live register map
            const std::string pc_offset = occurrence["pc_offset"].get<std::string>();
            auto reg_search_it = std::find_if(
                live_register_map[krn_name].begin(),
                live_register_map[krn_name].end(),
                [&](const auto &i) { return pc_offset == i.pcOffset; }
                );

            if (reg_search_it != live_register_map[krn_name].end()) {
                std::cout << "==== INFO :: Current VGPRs for the AMDGCN ISA instruction: " << reg_search_it->vgp_reg << std::endl;
                occurrence["used_register_count"] = reg_search_it->vgp_reg;

                if (reg_search_it->change_reg_from_last > 0) {
                    std::cout << "Increased VGPR pressure with " << std::abs(reg_search_it->change_vgpr_from_last) << " more registers compared to last AMGGCN ISA instruction" << std::endl;
                    occurrence["register_pressure_increase"] = std::abs(reg_search_it->change_vgpr_from_last);
                } else {
                    occurrence["register_pressure_increase"] = 0;
                }
            }
            */

            // TODO PC stall
        }

        if (occurrences.empty())
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== none of the registers can benefit from using __restrict__" << std::endl;
        }

    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];

    const auto static_result_file = static_result_path(assembly, "restrict");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a __restrict__ candidate exists.
     *  4. Preserve the result for the later full-analysis stage.
     *
     *  exit 0 -> restrict candidate detected
     *  exit 1 -> no restrict candidate detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto reg_map = parser_restrict(assembly);

        json static_result = build_static_restrict_result(reg_map);
        if (!has_restrict_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Could not save static restrict result to "
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
        std::cerr << "ERROR: Invalid arguments for restrict analysis." << std::endl;
        return 2;
    }

    json static_result;
    /*
     * Automatic mode: reuse the static result generated during detection.
     * Manual mode: no static result exists, therefore perform the original assembly parsing here.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto reg_map = parser_restrict(assembly);
        static_result = build_static_restrict_result(reg_map);
    }
    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    // live registers
    std::string livereg_dir = argv[3];
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map = live_registers_analysis(livereg_dir, assembly);

    bool save_as_json = std::strcmp(argv[4], "true") == 0;
    std::string json_out_dir = argv[5];

    auto result = analysis_restrict(static_result, mtc_map, live_register_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/restrict.json");
        json_file << result.dump(4);
        json_file.close();
    }
}
