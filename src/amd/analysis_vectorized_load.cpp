#include "parser_amdgcn_vectorized_load.hpp"
#include "parser_metrics.hpp"
#include "parser_liveregisters.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for the vectorized load analysis.
 * "result" contains the information that belongs to the existing final vectorized_load.json output.
 * "metadata" contains internal information required by later pipeline stages and is not be written to the final JSON output.
 */
json build_static_vectorized_load_result(const std::unordered_map<std::string, int>& ld_cnt_map, const std::unordered_map<std::string, std::vector<ld>>& ld_map)
{
    json static_result = {
        {"result", json::object()},
        {"metadata", json::object()}
    };

    for (const auto& [krn_name_1, ld_cnt] : ld_cnt_map)
    {
        if (krn_name_1 == "")
        {
            break;
        }

        json krn_result = {
            {"total", 0},
            {"occurrences", json::array()}
        };

        for (const auto& [krn_name_2, ld_vec] : ld_map)
        {
            if (krn_name_1 != krn_name_2)
            {
                continue;
            }

            for (const auto& ld_obj : ld_vec)
            {
                if (ld_obj.off.size() > 1 && ld_obj.size == "x1")
                {
                    krn_result["occurrences"].push_back({
                        {"severity", "WARNING"},
                        {"file_name", ld_obj.loc.file_name},
                        {"line_number", ld_obj.loc.line_num},
                        {"pc_offset", ld_obj.PC_offset},
                        {"register", ld_obj.vaddr_srsrc},
                        {"adjacent_memory_accesses", ld_obj.off.size()}
                    });
                }
                else
                {
                    krn_result["occurrences"].push_back({
                        {"severity", "INFO"},
                        {"file_name", ld_obj.loc.file_name},
                        {"line_number", ld_obj.loc.line_num},
                        {"pc_offset", ld_obj.PC_offset},
                        {"register", ld_obj.vaddr_srsrc},
                        {"register_load_type", ld_obj.size}
                    });
                }
            }
        }

        static_result["result"][krn_name_1] = krn_result;

        static_result["metadata"][krn_name_1] = {
            {"non_vectorized_load_count", ld_cnt}
        };
    }

    return static_result;
}

bool has_vectorized_load_candidate(const json& static_result)
{
    for (const auto& [krn_name, krn_result] : static_result["result"].items())
    {
        for (const auto& occurrence : krn_result["occurrences"])
        {
            if (occurrence["severity"].get<std::string>() == "WARNING")
            {
                return true;
            }
        }
    }

    return false;
}

json analysis_vectorized_load(
    json static_result,
    std::unordered_map<std::string, mtc> mtc_map,
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map)
{
    auto& result = static_result["result"];
    const auto& metadata = static_result["metadata"];

    for (auto& [krn_name, krn_result] : result.items())
    {
        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : vectorized load" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        int ld_cnt = 0;

        if (metadata.contains(krn_name))
        {
            ld_cnt = metadata[krn_name]["non_vectorized_load_count"].get<int>();
        }

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== total number of non-vectorized global vector load instances for this kernel "
                  << ld_cnt << std::endl;

        for (auto& occurrence : krn_result["occurrences"])
        {
            const std::string severity = occurrence["severity"].get<std::string>();
            const std::string file_name = occurrence["file_name"].get<std::string>();
            const int line_number = occurrence["line_number"].get<int>();
            const std::string reg = occurrence["register"].get<std::string>();
            const std::string pc_offset = occurrence["pc_offset"].get<std::string>();
            
            if (severity == "WARNING")
            {
                std::cout << std::endl;
                std::cout << "==== WARNING" << std::endl;
                std::cout << "==== load multiple dwords at once for register " << reg
                            << ", first used in a global load instruction in" << std::endl;
                std::cout << "     " << "file " << file_name << " at"
                            << " line " << line_number << " of your code" << std::endl;
                std::cout << "==== register " << reg << " has " << occurrence["adjacent_memory_accesses"].get<int>()
                            << " adjacent memory accesses" << std::endl;
            }
            else
            {
                const std::string load_type = occurrence["register_load_type"].get<std::string>();
                if (load_type == "x2")
                {
                    std::cout << std::endl;
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "==== global load instruction using register " << reg
                                << ", in file " << file_name << " at line number "
                                << line_number << " of your code, is already loading 2 double words at"
                                << " once" << std::endl;
                }
                if (load_type == "x3")
                {
                    std::cout << std::endl;
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "==== global load instruction using register " << reg
                                << ", in file " << file_name << " at line number "
                                << line_number << " of your"
                                << " code, is already loading 3 double words at once" << std::endl;
                }
                if (load_type == "x4")
                {
                    std::cout << std::endl;
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "==== global load instruction using register " << reg
                                << ", in file "
                                << file_name << " at line number " << line_number << " of your"
                                << " code, is already loading 4 double words at once" << std::endl;
                }
            }

            // --------- Register Pressure ------------------
            // search for a match between the PC offset of the current local load instruction and the PC offset of
            // an entry in the live register map
            auto reg_search_it = std::find_if(
                live_register_map[krn_name].begin(),
                live_register_map[krn_name].end(),
                [&](const auto &i) { return pc_offset == i.pcOffset; }
                );

            if (reg_search_it != live_register_map[krn_name].end()) 
            {
                std::cout << "==== INFO :: Current VGPRs for the AMDGCN ISA instruction: " << reg_search_it->vgp_reg << std::endl;
                occurrence["used_register_count"] = reg_search_it->vgp_reg;

                if (reg_search_it->change_vgpr_from_last > 0) {
                    std::cout << "Increased VGPR pressure with " << std::abs(reg_search_it->change_vgpr_from_last) << " more re compared to last AMGGCN ISA instruction" << std::endl;
                    occurrence["register_pressure_increase"] = std::abs(reg_search_it->change_vgpr_from_last);
                } else {
                    occurrence["register_pressure_increase"] = 0;
                }
            }

            // TODO PC stall
        }

        auto mtc_obj = mtc_map[krn_name];

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;
    }
    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];

    auto static_result_file = static_result_path(assembly, "vectorized_load");

    /*! Static detection mode:
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a vectorized load candidate exists.
     *  4. Preserve the static result for the later full-analysis stage.
     *
     *  exit 0 -> vectorized load candidate detected
     *  exit 1 -> no vectorized load candidate detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto tuple = parser_vectorized_load(assembly);
        auto ld_cnt_map = std::get<0>(tuple);
        auto ld_map = std::get<1>(tuple);

        json static_result =  build_static_vectorized_load_result(ld_cnt_map, ld_map);
        if (!has_vectorized_load_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Failed to save static vectorized load result."
                      << std::endl;
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
        std::cerr << "ERROR: Invalid arguments for vectorized load analysis." << std::endl;
        return 2;
    }

    json static_result;

    /*!
     * Automatic mode: reuse the result generated during static detection.
     * Manual mode: no static cache exists, therefore parse once here.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto tuple = parser_vectorized_load(assembly);

        auto ld_cnt_map = std::get<0>(tuple);
        auto ld_map = std::get<1>(tuple);

        static_result = build_static_vectorized_load_result(ld_cnt_map, ld_map);
    }

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    // live registers
    std::string livereg_dir = argv[3];
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map = live_registers_analysis(livereg_dir, assembly);

    bool save_as_json = std::strcmp(argv[4], "true") == 0;
    std::string json_out_dir = argv[5];

    json result = analysis_vectorized_load(static_result, mtc_map, live_register_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/vectorization.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
