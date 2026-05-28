#include "parser_amdgcn_deadlock_detection.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

json analysis_deadlock_detection(std::unordered_map<std::string, bool> deadlock_map)
{
    json result;

    for (auto [krn_name, deadlock] : deadlock_map)
    {
        json krn_result;
        krn_result["metrics"] = {
            {"deadlock_detect_flag", deadlock}
        };

        // TODO check if this is happening
        if (krn_name == "")
        {
            break;
        }

	std::cout << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;
        std::cout << "==== analysis    : deadlock detection" << std::endl;
        std::cout << "==== kernel name : " << krn_name << std::endl;
        std::cout << "======================================================================"
                  << "================================" << std::endl;

	std::cout << std::endl;
        std::cout << "==== INFO" << std::endl;
        if (deadlock)
        {
            std::cout << "==== deadlock in kernel could be possible" << krn_name << std::endl;
        }
        else
        {
            std::cout << "==== no possibility for deadlock detected in kernel " << krn_name << std::endl;
        }

        result[krn_name] = krn_result;
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    std::unordered_map<std::string, bool> deadlock_map = parser_deadlock_detection(assembly);

    int save_as_json = std::strcmp(argv[2], "true") == 0;
    std::string json_out_dir = argv[3];

    json result = analysis_deadlock_detection(deadlock_map);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/deadlock_detection.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
