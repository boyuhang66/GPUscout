#include "parser_amdgcn_datatype_conversion.hpp"
#include "parser_metrics.hpp"
#include "utilities/json.hpp"

using json = nlohmann::json;

json analysis_datatype_conversion(
    const std::unordered_map<std::string, conv>& conv_map,
    std::unordered_map<std::string, mtc> mtc_map)
{
    json result;

    for (const auto& [krn_name, conv_obj] : conv_map)
    {
        json krn_result = {
            {"occurrences", json::array()}
        };

        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
        }

	std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : datatype conversion" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        if (conv_obj.F2F_cnt > 0)
        {
	    std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << conv_obj.F2F_cnt << " F2F conversions found at the following locations:"
                      << std::endl;

            for (const auto& i : conv_obj.F2F_line)
            {
                std::cout << "     file name " << std::get<0>(i).file_name << " line " << std::get<0>(i).line_num
                          << std::endl;

                krn_result["occurrences"].push_back({
                    {"file_name", std::get<0>(i).file_name},
                    {"line_number", std::get<0>(i).line_num},
                    {"pc_offset", std::get<1>(i)},
                    {"type", "F2F"}
                });
            }
        }
        else
        {
	    std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no F2F conversions found" << std::endl;
        }

        if (conv_obj.I2F_cnt > 0)
        {
	    std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << conv_obj.I2F_cnt << " I2F conversions found at the following locations:"
                      << std::endl;

            for (const auto& i : conv_obj.I2F_line)
            {
                std::cout << "     file name " << std::get<0>(i).file_name << " line " << std::get<0>(i).line_num
                          << std::endl;

                krn_result["occurrences"].push_back({
                    {"file_name", std::get<0>(i).file_name},
                    {"line_number", std::get<0>(i).line_num},
                    {"pc_offset", std::get<1>(i)},
                    {"type", "I2F"}
                });
            }
        }
        else
        {
	    std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no I2F conversions found" << std::endl;
        }

        if (conv_obj.F2I_cnt > 0)
        {
	    std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << conv_obj.F2F_cnt << " F2I conversions found at the following locations:"
                      << std::endl;

            for (const auto& i : conv_obj.F2I_line)
            {
                std::cout << "     file name " << std::get<0>(i).file_name << " line " << std::get<0>(i).line_num
                          << std::endl;

                krn_result["occurrences"].push_back({
                    {"file_name", std::get<0>(i).file_name},
                    {"line_number", std::get<0>(i).line_num},
                    {"pc_offset", std::get<1>(i)},
                    {"type", "F2I"}
                });
            }
        }
        else
        {
	    std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no F2I conversions found" << std::endl;
        }

        auto mtc_obj = mtc_map[krn_name];

	std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        std::cout << "==== total number of type conversion instructions (such as converting data to or from F32 to F64)"
                  << std::endl;
        std::cout << "     issued to the VALU" << std::endl;
        std::cout << "     " << mtc_obj.ID_10_2_14 << std::endl;

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    auto conv_map = parser_datatype_conversion(assembly);

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_datatype_conversion(conv_map, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/datatype_conversion.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
