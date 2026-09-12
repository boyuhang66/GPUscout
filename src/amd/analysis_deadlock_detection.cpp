#include "parser_amdgcn_deadlock_detection.hpp"
#include "amd_helper.hpp"
#include "../utilities/json.hpp"

using json = nlohmann::json;

/*!
 * Build the reusable static result for deadlock detection.
 * The result contains all information required by the final deadlock_detection.json output
 */
json build_static_deadlock_detection_result(const std::unordered_map<std::string, bool>& deadlock_map)
{
    json static_result = { {"result", json::object()}};

    for (const auto& [krn_name, deadlock] : deadlock_map)
    {
        // TODO: check if this is happening
        if (krn_name.empty())
        {
            break;
        }

        json krn_result;

        krn_result["metrics"] = { {"deadlock_detect_flag", deadlock}};

        static_result["result"][krn_name] = krn_result;
    }

    return static_result;
}

/*!
 * Return true if at least one possible deadlock was detected.
 */
bool has_deadlock_candidate(const json& static_result)
{
    for (const auto& [krn_name, krn_result] : static_result["result"].items())
    {
        const bool deadlock = krn_result["metrics"]["deadlock_detect_flag"].get<bool>();

        if (deadlock)
        {
            return true;
        }
    }

    return false;
}

json analysis_deadlock_detection(json static_result)
{
    auto& result = static_result["result"];

    for (const auto& [krn_name, krn_result] : result.items())
    {
        const bool deadlock = krn_result["metrics"]["deadlock_detect_flag"].get<bool>();

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
    }

    return result;
}

int main(int argc, char **argv)
{
    std::string assembly = argv[1];
    const auto static_result_file = static_result_path(assembly, "deadlock_detection");

    /*! Static detection mode:
     *
     *  1. Parse AMDGCN assembly.
     *  2. Build the reusable static result.
     *  3. Detect whether a possible deadlock exists.
     *  4. Preserve the result for the later full-analysis stage.
     *
     *  exit 0 -> deadlock candidate detected
     *  exit 1 -> no deadlock candidate detected
     *  exit 2 -> error
     */
    if (argc == 3 && std::strcmp(argv[2], "--detect-only") == 0)
    {
        std::unordered_map<std::string, bool> deadlock_map = parser_deadlock_detection(assembly);
        json static_result = build_static_deadlock_detection_result(deadlock_map);

        if (!has_deadlock_candidate(static_result))
        {
            return 1;
        }

        if (!save_static_result(static_result_file, static_result))
        {
            std::cerr << "ERROR: Could not save static deadlock detection result to "
                      << static_result_file
                      << std::endl;

            return 2;
        }

        return 0;
    }

    /*! Full analysis mode:
     *  exit 0 -> successful analysis
     *  exit 2 -> invalid arguments
     */
    if (argc < 4)
    {
        std::cerr << "ERROR: Invalid arguments for deadlock detection analysis." << std::endl;
        return 2;
    }
    json static_result;

    /*
     * Automatic mode: reuse the static result generated during detection.
     * Manual mode: no static result exists, therefore perform the original assembly parsing here.
     */
    if (!load_static_result(static_result_file, static_result))
    {
        auto deadlock_map = parser_deadlock_detection(assembly);

        static_result = build_static_deadlock_detection_result(deadlock_map);
    }

    int save_as_json = std::strcmp(argv[2], "true") == 0;
    std::string json_out_dir = argv[3];

    json result = analysis_deadlock_detection(static_result);

    if (save_as_json)
    {
        std::ofstream json_file;
        json_file.open(json_out_dir + "/deadlock_detection.json");
        json_file << result.dump(4);
        json_file.close();
    }

    return 0;
}
