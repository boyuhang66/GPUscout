#ifndef PARSER_AMDGCN_WAVEFRONT_DIVERGENCE_HPP
#define PARSER_AMDGCN_WAVEFRONT_DIVERGENCE_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>

/// @brief the structure contains information about the source code location of instructions in the assembly
struct location
{
    int line_num;
    std::string file_name;
};

/// @brief struct representing a branch instruction
struct brc
{
    std::string tgt; // branch target
    location loc;
    std::string PC_offset; // PC offset of the branch instruction
};

/// @brief          assembly analysis, collects conditional branching instructions, and their targets  
/// @param filename assembly file
/// @return         tuple of two maps:
///                 - first map includes branch information
///                 - second map includes target branch line number
inline std::tuple<std::unordered_map<std::string, std::vector<brc>>, std::unordered_map<std::string, location>>
parser_wavefront_divergence(const std::string &filename)
{
    std::fstream file(filename, std::ios::in);
    std::string line;

    // map (key: kernel name | value: vector containing all occurrences of a branch instruction per kernel)
    std::unordered_map<std::string, std::vector<brc>> brc_map;
    // map (key: branch target | value: location of branch target)
    std::unordered_map<std::string, location> tgt_map;
    
    std::vector<brc> brc_vec;

    location loc_obj;

    std::string krn_name;
    bool set_lbl = false;
    std::string lbl;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                brc_vec.clear();

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_loc))
            {
                loc_obj.line_num = std::stoi(match[2].str());
                loc_obj.file_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_SOPP("(s_cbranch\\w*)"))) 
            {
                brc brc_obj;
                
                brc_obj.tgt = match[2].str();
                brc_obj.loc = loc_obj;
                brc_obj.PC_offset = match[match.size() - 2];

                brc_vec.push_back(brc_obj);
            }
            
            if (std::regex_search(line, match, regex_brc_lbl))
            {
                lbl = match[1].str();
                set_lbl = true;
            }

            if (set_lbl)
            {
                tgt_map[lbl] = loc_obj;
                set_lbl = false;
            }

            brc_map[krn_name] = brc_vec;
        }
        file.close();
    }
    else
    {
        std::cout << "==== ERROR" << std::endl;
        std::cout << "==== could not open the file " << filename << std::endl;
    }

    return std::make_tuple(brc_map, tgt_map);
}

#endif // PARSER_AMDGCN_WAVEFRONT_DIVERGENCE_HPP
