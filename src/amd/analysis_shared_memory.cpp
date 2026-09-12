#include "parser_amdgcn_shared_memory.hpp"
#include "parser_metrics.hpp"
#include "amd_helper.hpp"
#include <unordered_set>
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for the shared-memory analysis.
 * "result" keeps the existing final shared_memory.json structure.
 */
json build_static_shared_memory_result(const std::unordered_map<std::string, std::vector<reg>>& reg_map, const std::unordered_map<std::string, std::vector<brc>>& brc_map)
{
    json static_result = {
        {"result", json::object()},
    };

    for (const auto& [krn_name, reg_vec] : reg_map)
    {
        if (krn_name.empty())
        {
            break;
        }

        json krn_result = {
            {"occurrences", json::array()}
        };

        for (auto reg_obj : reg_vec)
        {
            // consider only registers with load count > 0, operation count > 1 and operation count > load count
            if (!((reg_obj.ld_count > 0) && (reg_obj.op_count > 1) && (reg_obj.op_count > reg_obj.ld_count)))
                continue;
        
            // -------------------------------------------------
            // Global loads that already use the LDS bit
            // -------------------------------------------------
            for (const auto& gbl_ld_obj : reg_obj.gbl_ld)
            {
                if (!gbl_ld_obj.lds_bit)
                {
                    continue;
                }

                json line_result = {
                    {"severity", "INFO"},
                    {"file_name", gbl_ld_obj.loc.file_name},
                    {"line_number", gbl_ld_obj.loc.line_num},
                    {"instruction_type", "global_load"},
                    {"register", reg_obj.reg_num},
                    {"uses_LDS_bit", true},
                    {"pc_offset", gbl_ld_obj.PC_offset}
                };

                krn_result["occurrences"].push_back(line_result);
            }

            // ------------------------------------------------------------
            // LDS writes
            // ------------------------------------------------------------
            for (const auto& shr_wr_obj : reg_obj.shr_wr)
            {
                json line_result = {
                    {"severity", "INFO"},
                    {"file_name", shr_wr_obj.loc.file_name},
                    {"line_number", shr_wr_obj.loc.line_num},
                    {"instruction_type", "lds_write"},
                    {"register", reg_obj.reg_num},
                    {"pc_offset", shr_wr_obj.PC_offset}
                };

                if (shr_wr_obj.cnt_to_shrd_mem_st > 0)
                {
                    line_result["instruction_count_to_shared_mem_store"] = shr_wr_obj.cnt_to_shrd_mem_st;
                }

                krn_result["occurrences"].push_back(line_result);
            }

            // ------------------------------------------------------------
            // Candidate global loads without the LDS bit
            // ------------------------------------------------------------
            for (const auto& gbl_ld_obj : reg_obj.gbl_ld)
            {
                if (gbl_ld_obj.lds_bit)
                {
                    continue;
                }

                // branch map stores an accumulative vector of branch instructions per branch

                // returns all conditional branch instructions encountered up to the end of the branch to which
                // the global load instruction belongs
                //for (auto j : brc_map[gbl_ld_obj.tgt_brc])
                //{
                    // if the branch instructions target the branch, the global load instruction belongs to
                    //if ((j.loc.line_num != 0) && (gbl_ld_obj.tgt_brc == j.tgt))
                bool inside_loop = false;
                auto brc_it = brc_map.find(krn_name);
                if (brc_it != brc_map.end())
                {
                    for (const auto& brc_obj : brc_it->second)
                    {
                        if (brc_obj.tgt == gbl_ld_obj.brc &&
                            std::stoi(brc_obj.PC_offset) >
                                std::stoi(gbl_ld_obj.PC_offset) &&
                            brc_obj.loop)
                        {
                            inside_loop = true;
                            break;
                        }
                    }
                }

                json line_result = {
                    {"severity", "WARNING"},
                    {"file_name", gbl_ld_obj.loc.file_name},
                    {"line_number", gbl_ld_obj.loc.line_num},
                    // Current candidate global-load instruction.
                    // {"pc_offset", gbl_ld_obj.PC_offset},
                    {"register", reg_obj.reg_num},
                    {"global_load_count", reg_obj.ld_count},
                    {"computation_instruction_count", reg_obj.op_count},
                    {"computation_instruction_pc_offsets", 0}, // TODO: parser currently does not preserve these PCs.
                    {"uses_shared_memory", false},
                    {"in_for_loop", inside_loop}
                };

                krn_result["occurrences"].push_back(line_result);
            }
        }

        static_result["result"][krn_name] = krn_result;
    }

    return static_result;
}

bool has_shared_memory_candidate(const json& static_result)
{
    for (const auto& [krn_name, krn_result] :
         static_result["result"].items())
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

json analysis_shared_memory(
    json static_result,
    std::unordered_map<std::string, mtc> mtc_map)
{
    auto& result = static_result["result"];

    for (auto& [krn_name, krn_result] : result.items())
    {
	    std::cout << std::endl;      
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : local memory" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        
        auto& occurrences = krn_result["occurrences"];
        bool shared_recommend_flag = false;

        /*
        * Used so that the register-level WARNING summary is printed
        * only once even when several candidate global-load instructions
        * belong to the same register.
        */
        std::unordered_set<std::string> warned_registers;

        for (auto& occurrence: occurrences)
        {
            const std::string severity = occurrence["severity"].get<std::string>();

            // INFO occurrences
            if (severity == "INFO")
            {
                const std::string instruction_type = occurrence["instruction_type"].get<std::string>();

                // Global load already using LDS bit
                if (instruction_type == "global_load")
                {
                    std::cout << std::endl;
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "==== register number " << occurrence["register"].get<std::string>()
                              << " is used by global load instruction that transfers data between LDS and"
                              << " memory instead of VGPRs and memory, in file " << occurrence["file_name"].get<std::string>()
                              << " at line number " << occurrence["line_number"].get<int>() << " of your code" << std::endl;
                }

                // Data is latter written to LDS
                else if (instruction_type == "lds_write")
                {
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "register number " << occurrence["register"].get<std::string>()
                              << " is storing data in local memory in file " << occurrence["file_name"].get<std::string>()
                              << " at line number " << occurrence["line_number"].get<int>() << " of your code" << std::endl;

                    if (occurrence.contains("instruction_count_to_shared_mem_store"))
                    {
                        std::cout << "==== data loaded from global memory is written to LDS after "
                                  << occurrence["instruction_count_to_shared_mem_store"].get<int>() << " instructions." << std::endl;
                        std::cout << "     using the LDS bit in global load instruction might help" << std::endl;
                    }
                }
                
                continue;
            }

            // Candidate global-load instruction
            if (severity == "WARNING" )
            {
                shared_recommend_flag = true;
                const std::string reg_num = occurrence["register"].get<std::string>();

                /*
                 * Print the register-level summary once.
                 *
                 * There may now be several WARNING occurrences for the same
                 * register because each candidate load is stored separately.
                 */
                if (warned_registers.insert(reg_num).second)
                {
                    std::cout << std::endl;
                    std::cout << "==== WARNING" << std::endl;
                    std::cout << "==== since the data at register number " << reg_num << " is accessed multiple "
                            << "times, you could benefit from using" << std::endl;
                    std::cout << "     local memory instead of global memory" << std::endl;
                    std::cout << "==== register number " << reg_num << " has " << occurrence["global_load_count"].get<int>()
                            << " total global load counts and " << occurrence["computation_instruction_count"].get<int>() << " computation "
                            << "instruction counts" << std::endl;
                    std::cout << "==== the following global load instruction (without LDS bit) use the register as vdst "
                            << "register" << std::endl;

                }

                std::cout << "==== global load instruction in file " << occurrence["file_name"].get<std::string>()
                        << " at line " << occurrence["line_number"].get<int>() << std::endl;
                
                if (occurrence["in_for_loop"].get<bool>())
                {
                    std::cout << "==== this global load instruction could be in a loop and "
                            << "hence could perform multiple load operations" << std::endl;

                    // TODO PC stall
                }
            }
        }

        if (!shared_recommend_flag)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no global loads found in the kernel which can benifit from using local memory"
                      << std::endl;
        }

        auto mtc_obj = mtc_map[krn_name];

        std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;
        std::cout << "==== indicates what percent of the kernel’s duration the LDS was actively executing instructions"
                  << std::endl;
        std::cout << "     (including, but not limited to, load, store and atomic operations"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_12_1_0 << std::endl;
        std::cout << "==== indicates the percentage of SIMDs in the VALU actively issuing LDS instructions, averaged "
                     "over" << std::endl;
        std::cout << "     the lifetime of the kernel"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_12_1_1 << std::endl;
        std::cout << "==== indicates the percentage of active LDS cycles that were spent servicing bank conflicts"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_12_1_3 << std::endl;

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    const auto static_result_file = static_result_path(assembly, "shared_memory");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a shared-memory candidate exists.
     *  4. Preserve the result for the later full-analysis stage.
     *
     *  exit 0 -> shared-memory candidate detected
     *  exit 1 -> no shared-memory candidate detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto tuple = parser_shared_memory(assembly);
        auto reg_map = std::get<0>(tuple);
        auto brc_map = std::get<1>(tuple);

        json static_result = build_static_shared_memory_result(reg_map, brc_map);

        if (!has_shared_memory_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Could not save static shared memory result to "
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
        std::cerr << "ERROR: Invalid arguments for shared memory analysis." << std::endl;
        return 2;
    }

    json static_result;
    /*
     * Automatic mode: static detection already parsed the assembly, so reuse the intermediate static result.
     * Manual mode: no intermediate result exists, therefore parse the assembly once here and build the same static representation.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto tuple = parser_shared_memory(assembly);

        auto reg_map = std::get<0>(tuple);
        auto brc_map = std::get<1>(tuple);

        static_result = build_static_shared_memory_result(reg_map, brc_map);
    }

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_shared_memory(static_result, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/shared_memory.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
