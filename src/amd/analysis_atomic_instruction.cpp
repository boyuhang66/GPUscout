#include "parser_amdgcn_atomic_instruction.hpp"
#include "parser_metrics.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"
#include <iostream>

using json = nlohmann::json;

/*!
 * Build the reusable static result for the atomic-instruction analysis.
 * "result" contains the information that belongs to the existing final global_atomics.json output.
 * "metadata" contains internal information required by later pipeline stages and is not be written to the final JSON output.
 */
json build_static_atomic_instruction_result(const std::unordered_map<std::string, atom>& atom_map,
                                            const std::unordered_map<std::string, std::vector<brc>>& brc_map)
{
    json static_result = {
        {"result", json::object()},
        {"metadata", json::object()}
    };

    for (const auto& [krn_name, atom_obj] : atom_map)
    {
        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
        }

        json krn_result = {
            {"occurrences", json::array()},
            {"shared_atomics", atom_obj.num_s}
        };

        json occurrence_metadata = json::array();

        // for every occurring global atomic instruction in the kernel
        for (const auto& gbl_atom_obj : atom_obj.gbl_atom)
        {
            bool inside_loop = false;
            // loop through all branch instructions in the kernel
            auto brc_it = brc_map.find(krn_name);
            if (brc_it != brc_map.end())
            {
                for (const auto& brc_obj : brc_it->second)
                {
                    if (brc_obj.tgt == gbl_atom_obj.brc &&
                        std::stoi(brc_obj.PC_offset) > std::stoi(gbl_atom_obj.PC_offset) &&
                        brc_obj.loop)
                    {
                        inside_loop = true;
                    }
                }
            }

            krn_result["occurrences"].push_back({
                    {"severity", "INFO"},
                    {"file_name", gbl_atom_obj.loc.file_name},
                    {"line_number", gbl_atom_obj.loc.line_num},
                    {"in_for_loop", inside_loop},
                    {"is_global", true},
            });

            // Internal information corresponding to the occurrence at the same array index.
            occurrence_metadata.push_back({
                {"pc_offset", gbl_atom_obj.PC_offset}
            });
        }

        // Shared atomic instructions
        for (const auto &shr_atom_obj : atom_obj.shr_atom)
        {
            bool inside_loop = false;
            
            auto brc_it = brc_map.find(krn_name);
            if (brc_it != brc_map.end())
            {

                // loop through all branch instructions in the kernel
                for (const auto& brc_obj : brc_it->second)
                {
                    if (brc_obj.tgt == shr_atom_obj.brc &&
                        std::stoi(brc_obj.PC_offset) > std::stoi(shr_atom_obj.PC_offset) &&
                        brc_obj.loop)
                    {
                        inside_loop = true;
                    }
                }
            }

            krn_result["occurrences"].push_back({
                    {"severity", "INFO"},
                    {"file_name", shr_atom_obj.loc.file_name},
                    {"line_number", shr_atom_obj.loc.line_num},
                    {"in_for_loop", inside_loop},
                    {"is_global", false},
            });

            // Internal information corresponding to the occurrence at the same array index.
            occurrence_metadata.push_back({
                {"pc_offset", shr_atom_obj.PC_offset}
            });
        }

        static_result["result"][krn_name] = krn_result;
        static_result["metadata"][krn_name] = {
            {"global_atomics", atom_obj.num_g},
            {"occurrence_metadata", occurrence_metadata}
        };
    }

    return static_result;
}

/*!
 * Return true if at least one atomic instruction occurrence was found during static analysis.
 */
bool has_atomic_instruction_candidate(const json& static_result)
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

json analysis_atomic_instruction(
    json static_result,
    std::unordered_map<std::string, mtc> mtc_map)
{
    auto& result = static_result["result"];
    auto& metadata = static_result["metadata"];

    for (const auto& [krn_name, krn_result] : result.items())
    {
	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : atomic instruction" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        auto& occurrences = krn_result["occurrences"];
        const int global_atomics = metadata[krn_name]["global_atomics"].get<int>();
        const int shared_atomics = krn_result["shared_atomics"].get<int>();


        if (global_atomics > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== number of global atomic instructions: " << global_atomics << std::endl;

            // for every occurring global atomic instruction in the kernel
            for (const auto& occurrence : occurrences)
            {
                if (!occurrence["is_global"].get<bool>())
                {
                    continue;
                }

                std::cout << std::endl;
                std::cout << "==== INFO" << std::endl;
                std::cout << "==== global atomic instruction found in file "
                          << occurrence["file_name"].get<std::string>() << " at line number "
                          << occurrence["line_number"].get<int>() << " of your source code" << std::endl;

                if (occurrence["in_for_loop"].get<bool>())
                {
                    std::cout << "==== the atomic instruction could be inside a loop" << std::endl;
                }
            }
		        
        }
        else if (global_atomics == 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no global atomics detected in the assembly file" << std::endl;
        }

        if (shared_atomics > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== number of shared atomic instructions in the assembly file "
                      << shared_atomics << std::endl;
            
            for (const auto &occurrence : occurrences)
            {
                if (occurrence["is_global"].get<bool>())
                {
                    continue;
                }

                std::cout << std::endl;
                std::cout << "==== INFO" << std::endl;
                std::cout << "==== shared atomic instruction found in file "
                          << occurrence["file_name"].get<std::string>() << " at line number "
                          << occurrence["line_number"].get<int>() << " of your source code" << std::endl;

                if (occurrence["in_for_loop"].get<bool>())
                {
                    std::cout << "==== the atomic instruction could be inside a loop" << std::endl;
                }
            }   
        }
        else if (shared_atomics == 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no shared atomics detected in the assembly file" << std::endl;
        }

        // TODO PC stall

        // Metrics
        auto mtc_obj = mtc_map[krn_name];

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== total number of global & generic memory atomic (with and without return) instructions "
                  << "executed on" << std::endl;
        std::cout << "     all compute units on the accelerator" << std::endl;
        std::cout << "     " << mtc_obj.ID_16_3_3 << std::endl;
        std::cout << "==== total number of cycles spent on LDS atomics with return" << std::endl;
        std::cout << "     " << mtc_obj.ID_12_2_5 << std::endl;
        std::cout << "==== total number of atomic requests (with and without return) to the L2 from all clients"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_17_3_4 << std::endl;
        std::cout << "==== percent of write requests generated by the L2 cache that are atomic requests to any memory "
                  << std::endl;
        std::cout << "     location" << std::endl;
        std::cout << "     " << mtc_obj.ID_17_2_7 << std::endl;
        std::cout << "==== total number of L2 requests to Infinity Fabric to atomically update 32B or 64B of data in "
                     "any" << std::endl;
        std::cout << "     memory location" << std::endl;
        std::cout << "     " << mtc_obj.ID_17_6_10/*ID_17_5_10*/ << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];

    const auto static_result_file = static_result_path(assembly, "atomic_instruction");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the static part of the final analysis result.
     *  3. If a candidate exists, preserve the result for the later full-analysis stage.
     *
     *  exit 0 -> atomic instruction detected
     *  exit 1 -> no atomic instruction detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto tuple = parser_atomic_instruction(assembly);
        auto atom_map = std::get<0>(tuple);
        auto lbl_map = std::get<1>(tuple);

        json static_result = build_static_atomic_instruction_result(atom_map, lbl_map);
        if (!has_atomic_instruction_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Could not save static atomic instruction result to "
                      << static_result_file << std::endl;
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
        std::cerr << "ERROR: Invalid arguments for atomic instruction analysis."
                  << std::endl;
        return 2;
    }

    json static_result;
    /*
     * Automatic mode: reuse the static result generated during detection.
     * Manual mode: no static result exists, therefore perform the original assembly parsing here.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto tuple = parser_atomic_instruction(assembly);
        auto atom_map = std::get<0>(tuple);
        auto lbl_map = std::get<1>(tuple);

        static_result = build_static_atomic_instruction_result(atom_map, lbl_map);
    }

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_atomic_instruction(static_result, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/global_atomics.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
