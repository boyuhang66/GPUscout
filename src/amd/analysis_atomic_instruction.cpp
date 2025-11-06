#include "parser_amdgcn_atomic_instruction.hpp"
#include "parser_metrics.hpp"
#include "utilities/json.hpp"
#include <iostream>

using json = nlohmann::json;

json analysis_atomic_instruction(
    const std::unordered_map<std::string, atom>& atom_map,
    std::unordered_map<std::string, std::vector<brc>> brc_map,
    std::unordered_map<std::string, mtc> mtc_map)
{
    json result;

    for (const auto& [krn_name, atom_obj] : atom_map)
    {
        json krn_result = {
            {"occurrences", json::array()},
        };

        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : atomic instruction" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        krn_result["shared_atomics"] = atom_obj.num_s;

        if (atom_obj.num_g > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== number of global atomic instructions: " << atom_obj.num_g << std::endl;

            // for every occurring global atomic instruction in the kernel
            for (const auto& gbl_atom_obj : atom_obj.gbl_atom)
            {
		        std::cout << std::endl;
                std::cout << "==== INFO" << std::endl;
                std::cout << "==== global atomic instruction found in file "
                << gbl_atom_obj.loc.file_name << " at line " << gbl_atom_obj.loc.line_num
                << " of your source code" << std::endl;

                bool inside_loop = false;
                // loop through all branch instructions in the kernel
                for (const auto& brc_obj : brc_map[krn_name])
                {
                    if (brc_obj.tgt == gbl_atom_obj.brc &&
                        std::stoi(brc_obj.PC_offset) > std::stoi(gbl_atom_obj.PC_offset) &&
                        brc_obj.loop)
                    {
                        inside_loop = true;
                    }
                }

                if (inside_loop)
                {
                    std::cout << "==== the atomic instruction could be inside a loop" << std::endl;
                }
                                
                krn_result["occurrences"].push_back({
                    {"file_name", gbl_atom_obj.loc.file_name},
                    {"line_number", gbl_atom_obj.loc.line_num},
                    {"in_for_loop", inside_loop},
                    {"is_global", true},
                });
            }
        }
        else if (atom_obj.num_g == 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no global atomics detected in the assembly file" << std::endl;
        }

        krn_result["shared_atomics"] = atom_obj.num_s;
        
        if (atom_obj.num_s > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== number of shared atomic instructions in the assembly file "
                      << atom_obj.num_s << std::endl;
            
            for (const auto &shr_atom_obj : atom_obj.shr_atom)
            {
		        std::cout << std::endl;
                std::cout << "==== INFO" << std::endl;
                std::cout << "==== shared atomic instruction found in file " << shr_atom_obj.loc.file_name
                          << " at line number " << shr_atom_obj.loc.line_num  << " of your source code. "
                          << std::endl;
                                
                bool inside_loop = false;
                // loop through all branch instructions in the kernel
                for (const auto& brc_obj : brc_map[krn_name])
                {
                    if (brc_obj.tgt == shr_atom_obj.brc &&
                        std::stoi(brc_obj.PC_offset) > std::stoi(shr_atom_obj.PC_offset) &&
                        brc_obj.loop)
                    {
                        inside_loop = true;
                    }
                }

                if (inside_loop)
                {
                    std::cout << "==== the atomic instruction could be inside a loop" << std::endl;
                }

                krn_result["occurrences"].push_back({
                    {"file_name", shr_atom_obj.loc.file_name},
                    {"line_number", shr_atom_obj.loc.line_num},
                    {"in_for_loop", inside_loop},
                    {"is_global", false},
                });
            }
        }
        else if (atom_obj.num_s == 0)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no shared atomics detected in the assembly file" << std::endl;
        }

        // TODO PC stall

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
        std::cout << "     " << mtc_obj.ID_17_5_10 << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    auto tuple = parser_atomic_instruction(assembly);
    auto atom_map = std::get<0>(tuple);
    auto lbl_map = std::get<1>(tuple);

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_atomic_instruction(atom_map, lbl_map, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/atomic_instruction.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
