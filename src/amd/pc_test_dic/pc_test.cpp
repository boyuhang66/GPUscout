//
// Created by Lukas on 28.03.26.
//

#include <string>
#include "../parser_pcsampling.hpp"

// Tests the pc_sampling with a example sample output (taken out of the documentation)

int main() {
    std::string pc_path = ".";
    std::string assembly_path = "./assembly.s";

    std::unordered_map<std::string, std::vector<pc_issue_samples>> warps = get_warp_stalls(pc_path, assembly_path);

    std::cout << "Warps size: " << warps.size() << std::endl;
}