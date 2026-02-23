#include "parser_amdgcn_restrict.hpp"
#include "parser_metrics.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

json analysis_restrict(
    const std::unordered_map<std::string, std::vector<reg>>& reg_map,
    const std::unordered_map<std::string, mtc>& mtc_map)
{
    json result;

    for (const auto& [krn_name, reg_vec] : reg_map)
    {
        json krn_result = {
            {"occurrences", json::array()}
        };
        
        if (krn_name == "")
        {
            break;
        }

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : __restrict__" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        bool benefit = false;

        for (auto reg_obj : reg_vec)
        {
            json line_result = {};
            
            if (!reg_obj.is_used)
            {
                std::cout << std::endl; 
		        std::cout << "==== INFO" << std::endl;
                std::cout << "==== register " << reg_obj.reg_num << ", first appearing used by global load in file "
                          << reg_obj.loc.file_name << " at line " << reg_obj.loc.line_num << " of your" << std::endl;
                std::cout << "     code, could benefit from using __restrict__" << std::endl;
                    
                benefit = true;

                line_result = {
                    {"file_name", reg_obj.loc.file_name},
                    {"line_number", reg_obj.loc.line_num},
                    {"register", reg_obj.reg_num}
                };

                // TODO PC stall

            }
            if (!line_result.is_null())
            {
                krn_result["occurrences"].push_back(line_result);
            }
        }

        if (!benefit)
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== none of the registers can benefit from using __restrict__" << std::endl;
        }

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    auto reg_map = parser_restrict(assembly);

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    auto save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    auto result = analysis_restrict(reg_map, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/restrict.json");
        json_file << result.dump(4);
        json_file.close();
    }
}
