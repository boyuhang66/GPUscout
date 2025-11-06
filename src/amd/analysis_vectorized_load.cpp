#include "parser_amdgcn_vectorized_load.hpp"
#include "parser_metrics.hpp"
#include "utilities/json.hpp"

using json = nlohmann::json;

json analysis_vectorized_load(
    const std::unordered_map<std::string, int>& ld_cnt_map,
    const std::unordered_map<std::string, std::vector<ld>>& ld_map,
    std::unordered_map<std::string, mtc> mtc_map)
{
    json result;

    for (const auto& [krn_name_1, ld_cnt] : ld_cnt_map)
    {
        json krn_result = {
            {"occurrences", json::array()}
        };

        // TODO check if this is happening
        if (krn_name_1 == "")
        {
            break;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : vectorized load" << std::endl;
        std::cout << "==== kernel name : " << krn_name_1 << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== total number of non-vectorized global vector load instances for this kernel "
                  << ld_cnt << std::endl;

        for (const auto& [krn_name_2, ld_vec] : ld_map)
        {
            if (krn_name_1 == krn_name_2)
            {
                for (auto ld_obj : ld_vec)
                {
                    json line_result;

                    if ((ld_obj.off.size() > 1) && (ld_obj.size == "x1"))
                    {
			            std::cout << std::endl;
                        std::cout << "==== WARNING" << std::endl;
                        std::cout << "==== load multiple dwords at once for register " << ld_obj.vaddr_srsrc
                                  << ", first used in a global load instruction in" << std::endl;
                        std::cout << "     " << "file " << ld_obj.loc.file_name << " at"
                                  << " line " << ld_obj.loc.line_num << " of your code" << std::endl;
                        std::cout << "==== register " << ld_obj.vaddr_srsrc << " has " << ld_obj.off.size()
                                  << " adjacent memory accesses" << std::endl;

                        line_result = {
                            {"file_name", ld_obj.loc.file_name},
                            {"line_number", ld_obj.loc.line_num},
                            {"pc_offset", ld_obj.PC_offset},
                            {"register", ld_obj.vaddr_srsrc},
                            {"adjacent_memory_accesses", ld_obj.off.size()}
                        };
                    }
                    else
                    {
                        if (ld_obj.size == "x2")
                        {
			                std::cout << std::endl;
                            std::cout << "==== INFO" << std::endl;
                            std::cout << "==== global load instruction using register " << ld_obj.vaddr_srsrc
                                      << ", in file " << ld_obj.loc.file_name << " at line number "
                                      << ld_obj.loc.line_num << " of your code, is already loading 2 double words at"
                                      << " once" << std::endl;
                        }
                        if (ld_obj.size == "x3")
                        {
			                std::cout << std::endl;
                            std::cout << "==== INFO" << std::endl;
                            std::cout << "==== global load instruction using register " << ld_obj.vaddr_srsrc
                                      << ", in file " << ld_obj.loc.file_name << " at line number "
                                      << ld_obj.loc.line_num << " of your"
                                      << " code, is already loading 3 double words at once" << std::endl;
                        }
                        if (ld_obj.size == "x4")
                        {
		                    std::cout << std::endl;
                            std::cout << "==== INFO" << std::endl;
                            std::cout << "==== global load instruction using register " << ld_obj.vaddr_srsrc
                                      << ", in file "
                                      << ld_obj.loc.file_name << " at line number " << ld_obj.loc.line_num << " of your"
                                      << " code, is already loading 4 double words at once" << std::endl;
                        }

                        line_result = {
                            {"file_name", ld_obj.loc.file_name},
                            {"line_number", ld_obj.loc.line_num},
                            {"pc_offset", ld_obj.PC_offset},
                            {"register", ld_obj.vaddr_srsrc },
                            {"register_load_type", ld_obj.size}
                        };
                    }

                    // TODO PC stall and register pressure

                    if (!line_result.is_null())
                    {
                        krn_result["occurrences"].push_back(line_result);
                    }
                }
            }
        }

        auto mtc_obj = mtc_map[krn_name_1];

	    std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== number of cycles a wavefront in the kernel dispatch stalled waiting on memory of any kind"
                  << std::endl;
        std::cout << "     " << mtc_obj.ID_7_2_4 << std::endl;

        result[krn_name_1] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    auto tuple = parser_vectorized_load(assembly);
    auto ld_cnt_map = std::get<0>(tuple);
    auto ld_map = std::get<1>(tuple);

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_vectorized_load(ld_cnt_map, ld_map, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/vectorized_load.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
