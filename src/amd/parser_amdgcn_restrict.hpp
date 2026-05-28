#ifndef PARSER_AMDGCN_RESTRICT_HPP
#define PARSER_AMDGCN_RESTRICT_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <tuple>
#include <set>

/// @brief the structure contains information about the source code location of instructions in the assembly
struct location
{
    int line_num;
    std::string file_name;
};

struct reg
{
    bool is_used;        // indicates if the register is used for anything other than loading data from
                         // global memory
    std::string reg_num; // register number
    location loc;             // source code line number corresponding to the assembly instruction that uses
                         // this register
    std::string PC_offset;
};

/// @brief          detect registers that are only used to load data from global memory
/// @param filename assembly file
/// @return         map storing register information for registers that load data from global memory
inline std::unordered_map<std::string, std::vector<reg>>
parser_restrict(const std::string &filename)
{
    std::fstream file(filename, std::ios::in);
    std::unordered_map<std::string, std::vector<reg>> reg_map;
    std::vector<reg> reg_vec;

    location loc_obj;
    std::string krn_name;

    std::string line;
    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                reg_vec.clear();

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_loc))
            {
                loc_obj.line_num = std::stoi(match[2].str());
                loc_obj.file_name = match[1].str();
            }

            RegexKind hit = RegexKind::NONE;
            /*
            if (std::regex_search(line, match, regex_FLAT("(global_load\\w*)")) ||
                std::regex_search(line, match, regex_MIMG("(image_load\\w*)", "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MUBUF("(buffer_load\\w*)", "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MTBUF("(tbuffer_load_format\\w*)",
                                                           "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)")))
            */
            if (std::regex_search(line, match, regex_FLAT("(global_load\\w*)"))
                    ? (hit = RegexKind::FLAT, true)
                : std::regex_search(line, match, regex_MIMG("(image_load\\w*)", "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)"))
                    ? (hit = RegexKind::MIMG, true)
                : std::regex_search(line, match, regex_MUBUF("(buffer_load\\w*)", "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)"))
                    ? (hit = RegexKind::MUBUF, true)
                : std::regex_search(line, match, regex_MTBUF("(tbuffer_load_format\\w*)", "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)"))
                    ? (hit = RegexKind::MTBUF, true)
                : false)
            {
                std::string vdst = match[2].str();

                auto it = std::find_if(
                    reg_vec.begin(),
                    reg_vec.end(),
                    [&](const auto &i) { return vdst == i.reg_num; }
                );

                if (it == reg_vec.end())
                {
                    reg reg_obj;
                    reg_obj.is_used = false;
                    reg_obj.reg_num = vdst;
                    reg_obj.loc = loc_obj;

                    // set PCoffset relative to captured instruction type because of different group
                    // Group in regex - Flat: 5, Mimg: 5, Mubuf: 4, Mtbuf: 4
                    int pc_index = 0;
                    switch (hit) {
                        case RegexKind::FLAT:
                            pc_index = 6;
                            break;
                        case RegexKind::MIMG:
                            pc_index = 8;
                            break;
                        case RegexKind::MUBUF:
                            pc_index = 7;
                            break;
                        case RegexKind::MTBUF:
                            pc_index = 7;
                            break;
                        default: break;
                    }
                    reg_obj.PC_offset = match[pc_index].str();

                    reg_vec.push_back(reg_obj);
                }
            }

            // TODO same logic
            if (std::regex_search(line, match, regex_VOP1("((?:v_rcp|v_sqrt|v_rsq|v_sin|v_cos|v_log|v_exp)\\w*)")) ||
                std::regex_search(line, match, regex_VOP2("((?:v_add|v_mul|v_mad)\\w*)")) ||
                std::regex_search(line, match, regex_VOP3("((?:v_add|v_mul|v_mad|v_fma|v_div_fma|v_rcp|v_sqrt|v_rsq"
                                                          "|v_sin|v_cos|v_log|v_exp)\\w*)")) ||
                std::regex_search(line, match, regex_VOP3P("((?:v_fma|v_mfma)\\w*)")) ||
                std::regex_search(line, match, regex_DS("((?:ds_add|ds_and|ds_dec|ds_inc|ds_max|ds_min|ds_or|ds_rsub"
                                                        "|ds_xor)\\w*)")) ||
                std::regex_search(line, match, regex_FLAT("((?:global_atomic)\\w*)")) ||
                std::regex_search(line, match, regex_MIMG("((?:image_atomic)\\w*)", "([\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MUBUF("((?:buffer_atomic)\\w*)", "([\\w\\[\\]:\\-]+)")))
            {
                std::string vdst = match[2].str();

                auto it = std::find_if(
                    reg_vec.begin(),
                    reg_vec.end(), 
                    [&](const auto &i) { return vdst == i.reg_num; }
                );

                if (it != reg_vec.end())
                {
                    it->is_used = true;
                }
            }

            reg_map[krn_name] = reg_vec;
        }
        file.close();
    }
    else
    {
        std::cout << "==== ERROR" << std::endl;
        std::cout << "====  could not open the file " << filename << std::endl;
    }

    return reg_map;
}

#endif // PARSER_AMDGCN_RESTRICT_HPP
