#include "parser_amdgcn_wavefront_divergence.hpp"
#include "parser_metrics.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for the wavefront divergence analysis.
 * "result" contains the information that belongs to the existing final wavefront_divergence.json output.
 */
json build_static_wavefront_divergence_result(const std::unordered_map<std::string, std::vector<brc>>& brc_map, const std::unordered_map<std::string, location>& tgt_map)
{
    json static_result = {
        {"result", json::object()}
    };

    for (const auto& [krn_name, brc_vec] : brc_map)
    {
        // TODO check if this is happening
        if (krn_name.empty())
        {
            continue;
        }

        json krn_result = {
            {"occurrences", json::array()}
        };

        for (const auto& brc_obj : brc_vec)
        {
            auto tgt_it = tgt_map.find(brc_obj.tgt);

            if (tgt_it == tgt_map.end())
            {
                continue;
            }

            // Branch instructions whose target maps to the same source location are not considered conditional branches.
            if (brc_obj.loc.file_name != tgt_it->second.file_name ||
                brc_obj.loc.line_num != tgt_it->second.line_num)
            {
                krn_result["occurrences"].push_back({
                    {"severity", "WARNING"},
                    {"pc_offset", brc_obj.PC_offset},
                    {"file_name", brc_obj.loc.file_name},
                    {"line_number", brc_obj.loc.line_num},
                    {"target_branch", brc_obj.tgt},
                    {"target_branch_start_file_name", tgt_it->second.file_name},
                    {"target_branch_start_line_number", tgt_it->second.line_num}
                });
            }
        }

        static_result["result"][krn_name] = krn_result;
    }

    return static_result;
}

bool has_wavefront_divergence_candidate(const json& static_result)
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

json analysis_wavefront_divergence(
    json static_result,
    std::unordered_map<std::string, mtc> mtc_map)
{
    auto& result = static_result["result"];

    for (auto& [krn_name, krn_result] : result.items())
    {
        // TODO check if this is happening
        if (krn_name.empty())
        {
            continue;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : wavefront divergence" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        for (auto& occurrence : krn_result["occurrences"])
        {
            const std::string file_name = occurrence["file_name"].get<std::string>();
            const int line_number = occurrence["line_number"].get<int>();
            const std::string target_branch = occurrence["target_branch"].get<std::string>();
            const std::string target_file_name = occurrence["target_branch_start_file_name"].get<std::string>();
            const int target_line_number = occurrence["target_branch_start_line_number"].get<int>();
            const std::string pc_offset = occurrence["pc_offset"].get<std::string>();

		    std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== conditional branching detected in file " << file_name
                      << "at line number " << line_number << " of your code, "
                      << "with target branch: " << target_branch << std::endl;
            std::cout << "     (target branch starts at in file " << target_file_name
                      << " at line number " << target_line_number << ")" << std::endl;

                // TODO PC stall
        }
            
        auto mtc_obj = mtc_map[krn_name];

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== total number of branch operations issued" << std::endl;
        std::cout << "     " << mtc_obj.ID_10_1_6 << std::endl;
        std::cout << "==== what percent of the kernel's duration the branch unit was busy executing instructions"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_11_2_5 << std::endl;

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    auto static_result_file = static_result_path(assembly, "wavefront_divergence");

    /*! Static detection mode:
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a wavefront divergence candidate exists.
     *  4. Preserve the static result for the later full-analysis stage.
     *
     *  exit 0 -> wavefront divergence candidate detected
     *  exit 1 -> no wavefront divergence candidate detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto tuple = parser_wavefront_divergence(assembly);
        auto brc_map = std::get<0>(tuple);
        auto tgt_map = std::get<1>(tuple);
        json static_result = build_static_wavefront_divergence_result(brc_map, tgt_map);

        if (!has_wavefront_divergence_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr
                << "ERROR: Failed to save static wavefront divergence result."
                << std::endl;
            return 2;
        }
        return 0;
    }

    /*! Full analysis mode:
     *  exit 0 -> successful analysis
     *  exit 2 -> invalid arguments
     */
    if (argc < 5)
    {
        std::cerr << "ERROR: Invalid arguments for wavefront divergence analysis." << std::endl;
        return 2;
    }

     json static_result;

    /*! Automatic mode: reuse static detection result.
     * Manual mode: no cache exists, therefore parse once here.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto tuple = parser_wavefront_divergence(assembly);

        auto brc_map = std::get<0>(tuple);
        auto tgt_map = std::get<1>(tuple);

        static_result = build_static_wavefront_divergence_result(brc_map, tgt_map);
    }

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_wavefront_divergence(static_result, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/warp_divergence.json");
        json_file << result.dump(4);
        json_file.close();
    }
}
