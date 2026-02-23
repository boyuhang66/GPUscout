#include "parser_amdgcn_shared_memory.hpp"
#include "parser_metrics.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

json analysis_shared_memory(
    const std::unordered_map<std::string, std::vector<reg>>& reg_map,
    std::unordered_map<std::string, std::vector<brc>> brc_map,
    std::unordered_map<std::string, mtc> mtc_map)
{
    json result;

    for (const auto& [krn_name, reg_vec] : reg_map)
    {
        json krn_result = {
            {"occurrences", {}}
        };

        bool shared_recommend_flag = false;

        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
       	}

	std::cout << std::endl;      
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : local memory" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        
        for (auto reg_obj : reg_vec)
        {
            json line_result;

            // consider only registers with load count > 0, operation count > 1 and operation count > load count
            if ((reg_obj.ld_count > 0) && (reg_obj.op_count > 1) && (reg_obj.op_count > reg_obj.ld_count))
            {
                for (auto gbl_ld_obj : reg_obj.gbl_ld)
                {
                    if (gbl_ld_obj.lds_bit)
                    {
                        shared_recommend_flag = false;
			
			std::cout << std::endl;
                        std::cout << "==== INFO" << std::endl;
                        std::cout << "==== register number " << reg_obj.reg_num
                                  << " is used by global load instruction that transfers data between LDS and"
                                  << " memory instead of VGPRs and memory, in file " << gbl_ld_obj.loc.file_name
                                  << " at line number " << gbl_ld_obj.loc.line_num << " of your code" << std::endl;

                        line_result = {
                            {"file_name", gbl_ld_obj.loc.file_name},
                            {"line_number", gbl_ld_obj.loc.line_num},
                            {"instruction_type", "global_load"},
                            {"register", reg_obj.reg_num},
                            {"uses_LDS_bit", true},
                            {"pc_offset", gbl_ld_obj.PC_offset}
                        };
                    }
                }

                for (auto shr_wr_obj : reg_obj.shr_wr)
                {
                    shared_recommend_flag = false;

		    std::cout << std::endl;
                    std::cout << "==== INFO" << std::endl;
                    std::cout << "register number " << reg_obj.reg_num
                              << " is storing data in local memory in file " << shr_wr_obj.loc.file_name
                              << " at line number " << shr_wr_obj.loc.line_num << " of your code" << std::endl;

                    line_result = {
                        {"file_name", shr_wr_obj.loc.file_name},
                        {"line_number", shr_wr_obj.loc.line_num},
                        {"instruction_type", "lds_write"},
                        {"register", reg_obj.reg_num},
                        {"pc_offset", shr_wr_obj.PC_offset}
                    };

                    if (shr_wr_obj.cnt_to_shrd_mem_st > 0)
                    {
                        std::cout << "==== data loaded from global memory is written to LDS after "
                                  << shr_wr_obj.cnt_to_shrd_mem_st << " instructions." << std::endl;
                        std::cout << "     using the LDS bit in global load instruction might help" << std::endl;

                        line_result["instruction_count_to_shared_mem_store"] = shr_wr_obj.cnt_to_shrd_mem_st;
                    }
                }

		std::cout << std::endl;
                std::cout << "==== WARNING" << std::endl;
                std::cout << "==== since the data at register number " << reg_obj.reg_num << " is accessed multiple "
                          << "times, you could benefit from using" << std::endl;
                std::cout << "     local memory instead of global memory" << std::endl;
                std::cout << "==== register number " << reg_obj.reg_num << " has " << reg_obj.ld_count
                          << " total global load counts and " << reg_obj.op_count << " computation "
                          << "instruction counts" << std::endl;
                std::cout << "==== the following global load instruction (without LDS bit) use the register as vdst "
                          << "register" << std::endl;

                for (auto gbl_ld_obj : reg_obj.gbl_ld)
                {
                    if (!gbl_ld_obj.lds_bit)
                    {
                        // branch map stores an accumulative vector of branch instructions per branch

                        // returns all conditional branch instructions encountered up to the end of the branch to which
                        // the global load instruction belongs
                        //for (auto j : brc_map[gbl_ld_obj.tgt_brc])
                        //{
                            // if the branch instructions target the branch, the global load instruction belongs to
                            //if ((j.loc.line_num != 0) && (gbl_ld_obj.tgt_brc == j.tgt))
                            //{
                                std::cout << "==== global load instruction in file " << gbl_ld_obj.loc.file_name
                                          << " at line " << gbl_ld_obj.loc.line_num << std::endl;

                                bool inside_loop = false;
                                // loop through all branch instructions in the kernel
                                for (const auto& brc_obj : brc_map[krn_name])
                                {
                                    if (brc_obj.tgt == gbl_ld_obj.brc &&
                                        std::stoi(brc_obj.PC_offset) > std::stoi(gbl_ld_obj.PC_offset) &&
                                        brc_obj.loop)
                                    {
                                        inside_loop = true;
                                    }
                                }

                                if (inside_loop)
                                {
                                    std::cout << "==== this global load instruction could be in a loop and "
                                              << "hence could perform multiple load operations" << std::endl;

                                    // TODO PC stall
                                }

                                line_result = {
                                    {"file_name", gbl_ld_obj.loc.file_name},
                                    {"line_number", gbl_ld_obj.loc.line_num},
                                    {"register", reg_obj.reg_num},
                                    {"global_load_count", reg_obj.ld_count},
                                    {"computation_instruction_count", reg_obj.op_count},
                                    {"uses_shared_memory", false},
                                    {"in_for_loop", inside_loop}
                                };

                                shared_recommend_flag = true;
                            //}
                        //}
                    }
                }
            }

            if (!line_result.is_null())
            {
                krn_result["occurrences"].push_back(line_result);
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
    
    auto tuple = parser_shared_memory(assembly);
    auto reg_map = std::get<0>(tuple);
    auto brc_map = std::get<1>(tuple);

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_shared_memory(reg_map, brc_map, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/shared_memory.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
