#include "parser_metrics.hpp"
#include "parser_liveregisters.hpp"
//#include "parser_pcsampling.hpp"
#include "../utilities/json.hpp"
#include "../utilities/helper.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

using json = nlohmann::json;

inline std::regex regex_source_file_path() {
    // ; /home/ge64cax2/GPUscout/examples/amd/register_spilling-vec.cpp:10
    return std::regex(
        "^"                                       // regex must match from the beginning of the string
        ";\\s"                                    // matches the line start "; "
        "(/[^\\s:]+)"                              // matches the path
        ":"                                       // matches the path / line number delimiter
        "(\\d*)"                                  // matches the line number
        ".*"                                      // matches anything following
    );
}

// Defines the register pressure json output
void to_json(json& j_obj, const live_registers& registers) {
    j_obj["pcOffset"] = registers.pcOffset;
    j_obj["vgp_reg"] = registers.vgp_reg;
    j_obj["sgp_reg"] = registers.sgp_reg;
}


int main(int argc, char **argv) {
    std::string json_files_dir = argv[1];
    std::string output_file_path = argv[2];
    std::string assembly_file = argv[3];
    std::string metrics_dir = argv[4];
    std::string livereg_dir = argv[5];
    //std::string sass_file = argv[3];
    //std::string sass_register_file = argv[4];
    //std::string ptx_file = argv[5];
    //std::string pc_samples_file = argv[6]; TODO PC Sampling
    // std::string metrics_file = argv[7];
    // int sm_count = std::stoi(argv[8]); TODO AMD equivalent?

    json result = {
        //{"kernels", json::object()}, TODO: Doppelt in Nvidia definiert?
        {"vendor", "amd"},
        {"analyses", json::object()},
        {"metrics", json::object()},
        {"stalls", json::object()},
        {"binary_files", {
                {"assembly", ""},
                //{"sass_registers", ""},
                //{"ptx", ""},
        }},
        {"register_pressure", json::object()},
        {"source_files", json::object()},
        {"kernels", json::object()} //TODO KERNEL in json - Z... und demangled Version. Auch bei Analysen demangled Version verwenden
    };

    // Add individual analysis results to result file
    for (const auto &file : std::filesystem::directory_iterator(json_files_dir))
    {
        if (file.is_directory())
            continue;
        std::string path = file.path();
        std::string filename = path.substr(path.find_last_of("/") + 1);

        if (filename.find(".json") == std::string::npos)
            continue;

        filename = filename.substr(0, filename.length() - 5); // cut .json
        std::ifstream analysis_file(path);
        if (analysis_file.is_open()) {
            result["analyses"][filename] = json::parse(analysis_file);

            // Add kernel mangled and demangled kernel names if not already present
            for (auto& kernel : result["analyses"][filename].items()) {
                if (!result["kernels"].contains(kernel.key())) {
                    result["kernels"][kernel.key()] = get_demangled_kernel(kernel.key(), "c++filt");
                }
            }
        }
        analysis_file.close();
    }





    // Add register pressure information - to_json function is used to transfer to json
    std::unordered_map<std::string, std::vector<live_registers>> live_register_map;
    live_register_map = live_registers_analysis(livereg_dir, assembly_file);
    json json_pressure = {};
    for (auto [kernel_name, vec_registers] : live_register_map) {
        json_pressure[kernel_name] = vec_registers;
    }
    result["register_pressure"] = json_pressure;

    // Add metrics
    std::unordered_map<std::string, mtc> metric_map = parser_metrics(metrics_dir, assembly_file);
    json json_metrics = {};
    for (auto [kernel_name, mtc_obj] : metric_map) {
        //json_metrics[kernel_name] = "test";// total_memory_flow(v_metric, sm_count); // TODO general metrics
        json_metrics[kernel_name]["misc"] = mtc_obj;
    }
    result["metrics"] = json_metrics;

    // Copy source files and save their mapping
    // Get source files used in ptx
    std::ifstream assembly_content(assembly_file);
    std::string file_content;

    if (assembly_content.is_open()) {
        std::string line;
        while (std::getline(assembly_content, line)) {
            file_content += line + '\n';

            // get source file paths
            std::smatch match;
            if (std::regex_search(line, match, regex_source_file_path())) {
                // Skip already added source files
                if (result["source_files"].find(match[1].str()) != result["source_files"].end()) {
                    continue;
                }

                std::ifstream source_file(match[1].str());
                if (source_file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(source_file)), (std::istreambuf_iterator<char>()));
                    result["source_files"][match[1].str()] = content;
                }
                source_file.close();
            }
            /*
            if (line.find("; /") != std::string::npos) {
                // ; /home/ge64cax2/GPUscout/examples/amd/register_spilling-vec.cpp:10
                std::istringstream ss(line);
                std::getline(ss, tmp_string, ' ');
                std::getline(ss, tmp_string, ':');

                //std::cout << tmp_string << '\n';

                // Skip already added source files
                if (result["source_files"].find(tmp_string) != result["source_files"].end()) {
                    continue;
                }

                std::ifstream source_file(tmp_string);
                if (source_file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(source_file)), (std::istreambuf_iterator<char>()));
                    result["source_files"][tmp_string] = content;
                }
                source_file.close();

            }
            */
        }
    }
    result["binary_files"]["assembly"] = file_content;
    assembly_content.close();


    // Create json output file and when one already exists add (<number>) to the filename
    std::ofstream result_file;
    std::string save_file_path = output_file_path;

    int index = 1;
    while (std::filesystem::exists(save_file_path + ".json")) {
        save_file_path = output_file_path + " (" + std::to_string(index++) + ")";
    }
    result_file.open(save_file_path + ".json");

    if (result_file.is_open()) {
        result_file << result.dump(4);
        result_file.close();
    }
}