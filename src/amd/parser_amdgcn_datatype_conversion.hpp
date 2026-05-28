#ifndef PARSER_AMDGCN_DATATYPE_CONVERSION_HPP
#define PARSER_AMDGCN_DATATYPE_CONVERSION_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

struct loc
{
    bool operator<(const loc& loc) const
    {
        return line_num < loc.line_num;
    }

    int line_num;
    std::string file_name;
};

/// @brief struct that holds information about various datatype conversions
struct conv
{
    int I2F_cnt = 0; // integer to floating point conversion
    int F2I_cnt = 0; // floating point to integer conversion
    int F2F_cnt = 0; // floating point to floating point conversion

    std::set<std::pair<loc, std::string>> I2F_line;
    std::set<std::pair<loc, std::string>> F2I_line;
    std::set<std::pair<loc, std::string>> F2F_line;
};

inline std::unordered_map<std::string, conv>
parser_datatype_conversion(const std::string &filename)
{
    std::string line;
    std::fstream file(filename, std::ios::in);

    std::unordered_map<std::string, conv> conv_map;

    conv conv_obj;
    loc loc_obj;

    std::string krn_name;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                conv_obj.F2F_cnt = 0;
                conv_obj.F2I_cnt = 0;
                conv_obj.I2F_cnt = 0;
                conv_obj.I2F_line.clear();
                conv_obj.F2I_line.clear();
                conv_obj.F2F_line.clear();

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_loc))
            {
                loc_obj.line_num = std::stoi(match[2].str());
                loc_obj.file_name = match[1].str();
            }

            // F2F
            if (std::regex_search(line, match, regex_VOP1("((?:v_cvt_f16_f32|v_cvt_f32_f16|v_cvt_f32_f64"
                                                          "|v_cvt_f64_f32)\\w*)")))
            {
                conv_obj.F2F_cnt++;
                conv_obj.F2F_line.insert(std::make_pair(loc_obj, match[match.size() - 2]));
            }

            // I2F
            if (std::regex_search(line, match, regex_VOP1("((?:v_cvt_f16_i16|v_cvt_f16_u16|v_cvt_f32_i32"
                                                          "|v_cvt_f32_u32|v_cvt_f64_i32|v_cvt_f64_u32)\\w*)")))
            {
                conv_obj.I2F_cnt++;
                conv_obj.I2F_line.insert(std::make_pair(loc_obj, match[match.size() - 2]));
            }

            // F2I
            if (std::regex_search(line, match, regex_VOP1("((?:v_cvt_flr_i32_f32|v_cvt_i16_f16|v_cvt_i32_f32"
                                                          "|v_cvt_i32_f64|v_cvt_u16_f16|v_cvt_u32_f32|v_cvt_u32_f64)"
                                                          "\\w*)")))
            {
                conv_obj.F2I_cnt++;
                conv_obj.F2I_line.insert(std::make_pair(loc_obj, match[match.size() - 2]));
            }

            conv_map[krn_name] = conv_obj;
        }
        file.close();
    }
    else
    {
        std::cout << "==== ERROR" << std::endl;
        std::cout << "==== could not open the file " << filename << std::endl;
    }

    return conv_map;
}

#endif // PARSER_AMDGCN_DATATYPE_CONVERSION_HPP
