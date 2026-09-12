#include "parser_amdgcn_datatype_conversion.hpp"
#include "parser_metrics.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for the datatype-conversion analysis.
 * "result" contains the information that belongs to the existing final datatype_conversion.json output.
 * "metadata" contains internal information required by later pipeline stages and is not written to the final JSON output.
 */
json build_static_datatype_conversion_result(const std::unordered_map<std::string, conv>& conv_map)
{
    json static_result = {
        {"result", json::object()},
        {"metadata", json::object()}
    };

    for (const auto& [krn_name, conv_obj] : conv_map)
    {
        // TODO: check if this is happening
        if (krn_name.empty())
        {
            break;
        }

        json krn_result = {
            {"occurrences", json::array()}
        };

        // F2F conversions
        for (const auto& conversion : conv_obj.F2F_line)
        {
            krn_result["occurrences"].push_back({
                {"severity", "WARNING"},
                {"file_name", std::get<0>(conversion).file_name},
                {"line_number", std::get<0>(conversion).line_num},
                {"pc_offset", std::get<1>(conversion)},
                {"type", "F2F"}
            });
        }

        // I2F conversions
        for (const auto& conversion : conv_obj.I2F_line)
        {
            krn_result["occurrences"].push_back({
                {"severity", "WARNING"},
                {"file_name", std::get<0>(conversion).file_name},
                {"line_number", std::get<0>(conversion).line_num},
                {"pc_offset", std::get<1>(conversion)},
                {"type", "I2F"}
            });
        }

        // F2I conversions
        for (const auto& conversion : conv_obj.F2I_line)
        {
            krn_result["occurrences"].push_back({
                {"severity", "WARNING"},
                {"file_name", std::get<0>(conversion).file_name},
                {"line_number", std::get<0>(conversion).line_num},
                {"pc_offset", std::get<1>(conversion)},
                {"type", "F2I"}
            });
        }

        /*
         * Keep the existing final JSON structure in "result".
         */
        static_result["result"][krn_name] = krn_result;

        static_result["metadata"][krn_name] = {
            {"F2F_count", conv_obj.F2F_cnt},
            {"I2F_count", conv_obj.I2F_cnt},
            {"F2I_count", conv_obj.F2I_cnt}
        };
    }

    return static_result;
}
        
/*!
 * Return true if at least one datatype-conversion candidate was found
 * during static analysis.
 */
bool has_datatype_conversion_candidate(const json& static_result)
{
    for (const auto& [krn_name, metadata] : static_result["metadata"].items())
    {
        const int F2F_count = metadata["F2F_count"].get<int>();

        const int I2F_count = metadata["I2F_count"].get<int>();

        const int F2I_count = metadata["F2I_count"].get<int>();

        if (F2F_count > 0 || I2F_count > 0 || F2I_count > 0)
        {
            return true;
        }
    }

    return false;
}

json analysis_datatype_conversion(
    json static_result,
    std::unordered_map<std::string, mtc> mtc_map)
{
    auto& result = static_result["result"];
    const auto& metadata = static_result["metadata"];

    for (auto& [krn_name, krn_result] : result.items())
    {

	    std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : datatype conversion" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

        const auto& occurrences = krn_result["occurrences"];

        const int F2F_count = metadata[krn_name]["F2F_count"].get<int>();

        const int I2F_count = metadata[krn_name]["I2F_count"].get<int>();

        const int F2I_count = metadata[krn_name]["F2I_count"].get<int>();
        
        // ---------------- F2F ----------------

        if (F2F_count > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << F2F_count << " F2F conversions found at the following locations:"
                      << std::endl;

            for (const auto& occurrence : occurrences)
            {
                if (occurrence["type"].get<std::string>() != "F2F")
                {
                    continue;
                }
                std::cout << "     file name " << occurrence["file_name"].get<std::string>() << " line " << occurrence["line_number"].get<int>()
                          << std::endl;
            }
        }
        else
        {
	        std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no F2F conversions found" << std::endl;
        }

        // ---------------- I2F ----------------

        if (I2F_count > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << I2F_count << " I2F conversions found at the following locations:"
                      << std::endl;

            for (const auto& occurrence : occurrences)
            {
                if (occurrence["type"].get<std::string>() != "I2F")
                {
                    continue;
                }
                std::cout << "     file name " << occurrence["file_name"].get<std::string>() << " line " << occurrence["line_number"].get<int>()
                          << std::endl;
            }
        }
        else
        {
	    std::cout << std::endl;
            std::cout << "==== INFO" << std::endl;
            std::cout << "==== no I2F conversions found" << std::endl;
        }

        // ---------------- F2I ----------------
        if (F2I_count > 0)
        {
	        std::cout << std::endl;
            std::cout << "==== WARNING" << std::endl;
            std::cout << "==== there are " << F2I_count << " F2I conversions found at the following locations:"
                      << std::endl;

            for (const auto& occurrence : occurrences)
            {
                if (occurrence["type"].get<std::string>() != "F2I")
                {
                    continue;
                }

                std::cout
                    << "     file name "
                    << occurrence["file_name"].get<std::string>()
                    << " line "
                    << occurrence["line_number"].get<int>()
                    << std::endl;
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
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    const auto static_result_file = static_result_path(assembly, "datatype_conversion");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a datatype-conversion candidate exists.
     *  4. Preserve the static result for the later full-analysis stage.
     *
     *  exit 0 -> datatype conversion detected
     *  exit 1 -> no datatype conversion detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        auto conv_map = parser_datatype_conversion(assembly);
        json static_result = build_static_datatype_conversion_result(conv_map);

        if (!has_datatype_conversion_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Could not save static datatype conversion result to "
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
        std::cerr << "ERROR: Invalid arguments for datatype conversion analysis."
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
        auto conv_map = parser_datatype_conversion(assembly);
        static_result = build_static_datatype_conversion_result(conv_map);
    }

    std::string mtc_dir = argv[2];
    auto mtc_map = parser_metrics(mtc_dir, assembly);

    int save_as_json = std::strcmp(argv[3], "true") == 0;
    std::string json_out_dir = argv[4];

    json result = analysis_datatype_conversion(static_result, mtc_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/datatype_conversion.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
