#ifndef PARSER_AMDGCN_VECTORIZED_HPP
#define PARSER_AMDGCN_VECTORIZED_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <regex>

struct location
{
    int line_num;
    std::string file_name;
};

struct ld
{
    std::string type;
    std::string vaddr_srsrc;        // register holding the memory address
    std::string size;               // size of the load instruction
    std::string PC_offset;          // PC_offset of the load instruction
    std::vector<unsigned long> off; // offset
    location loc;
};

inline std::tuple<std::unordered_map<std::string, int>, std::unordered_map<std::string, std::vector<ld>>>
parser_vectorized_load(const std::string &filename)
{
    std::fstream file(filename, std::ios::in);

    std::unordered_map<std::string, int> ld_cnt_map;
    std::unordered_map<std::string, std::vector<ld>> ld_map;

    std::vector<ld> ld_vec;
    location loc_obj;
    int ld_cnt; // counts total number of global load instructions
    std::string krn_name;

    std::string line;
    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                ld_cnt = 0;
                ld_vec.clear();

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_loc))
            {
                loc_obj.line_num = std::stoi(match[2].str());
                loc_obj.file_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_MUBUF("(buffer_load_dword(?:_(x[2-4]))?)",
                                                           "((?!s\\[0:3\\])[\\w\\[\\]:\\-]+)")))
            {
                ld_cnt++;

                std::string size;
                std::smatch size_match;
                if (std::regex_search(line, size_match, std::regex("buffer_load_dword_(x[2-4])")))
                {
                    size = size_match[1].str();
                }
                else
                {
                    size = "x1";
                }

                std::string srsrc = match[4].str();

                unsigned long off;
                std::smatch offset_match;
                if (std::regex_search(line, offset_match, std::regex("offset:(0x[0-9A-Fa-f]+|\\d+)")))
                {
                    off = std::stoul(offset_match[1].str());
                }
                // assume, that the only hex number in the instruction is a offset
                else if (std::regex_search(line, offset_match, std::regex("(0x[0-9A-Fa-f]+)")))
                {
                    off = std::stoul(offset_match[1].str());
                }
                else
                {
                    off = 0;
                }

                auto it = std::find_if(
                    ld_vec.begin(),
                    ld_vec.end(),
                    [&](const auto &i) { return loc_obj.line_num == i.loc.line_num &&
                                                     loc_obj.file_name == i.loc.file_name; });

                if (it != ld_vec.end())
                {
                    // There was another global load instruction seen at this code location
                    auto it = std::find_if(
                        ld_vec.begin(),
                        ld_vec.end(),
                        [&](const auto &i) { return ("buffer_load" == i.type) &&
                                                         (srsrc == i.vaddr_srsrc) &&
                                                         (loc_obj.line_num == i.loc.line_num) &&
                                                         (loc_obj.file_name == i.loc.file_name); }
                    );

                    // Check if the base (register) is present
                    if (it != ld_vec.end()) // base not present
                    {
                        // a global load instruction at the same location in the source code,
                        // targeting the same address register has already occurred
                        it->off.push_back(off);
                    }
                    else
                    {
                        // a global load instruction at the same location in the source code,
                        // targeting a different address register has already occurred,
                        // remember the encountered global load instruction
                        ld ld_obj;
                        ld_obj.type = "buffer_load";
                        ld_obj.off.push_back(off);
                        ld_obj.vaddr_srsrc = srsrc;
                        ld_obj.size = size;
                        ld_obj.PC_offset = match[match.size() - 2].str();
                        ld_obj.loc = loc_obj;
                        ld_vec.push_back(ld_obj);
                    }
                }
                else
                {
                    // no global load instruction has been seen in this code location yet,
                    // remember the encountered global load instruction
                    ld ld_obj;
                    ld_obj.type = "buffer_load";
                    ld_obj.off.push_back(off);
                    ld_obj.vaddr_srsrc = srsrc;
                    ld_obj.size = size;
                    ld_obj.PC_offset = match[match.size() - 2].str();
                    ld_obj.loc = loc_obj;
                    ld_vec.push_back(ld_obj);
                }
            }

            if (std::regex_search(line, match, regex_FLAT("(global_load_dword(?:_(x[2-4]))?)")))
            {
                ld_cnt++;

                std::string size;
                std::smatch size_match;
                if (std::regex_search(line, size_match, std::regex("global_load_dword_(x[2-4])")))
                {
                    size = size_match[1].str();
                }
                else
                {
                    size = "x1";
                }

                std::string vaddr = match[3].str();

                unsigned long off;
                std::smatch offset_match;
                if (std::regex_search(line, offset_match, std::regex("offset:(0x[0-9A-Fa-f]+|\\d+)")))
                {
                    off = std::stoul(offset_match[1].str());
                }
                // assume, that the only hex number in the instruction is a offset
                else if (std::regex_search(line, offset_match, std::regex("(0x[0-9A-Fa-f]+)")))
                {
                    off = std::stoul(offset_match[1].str());
                }
                else
                {
                    off = 0;
                }

                auto it = std::find_if(
                    ld_vec.begin(),
                    ld_vec.end(),
                    [&](const auto &i) { return loc_obj.line_num == i.loc.line_num &&
                                                     loc_obj.file_name == i.loc.file_name; });

                if (it != ld_vec.end())
                {
                    // There was another global load instruction seen at this code location
                    auto it = std::find_if(
                        ld_vec.begin(),
                        ld_vec.end(),
                        [&](const auto &i) { return ("global_load" == i.type) &&
                                                         (vaddr == i.vaddr_srsrc) &&
                                                         (loc_obj.line_num == i.loc.line_num) &&
                                                         (loc_obj.file_name == i.loc.file_name); }
                    );

                    // Check if the base (register) is present
                    if (it != ld_vec.end()) // base not present
                    {
                        // a global load instruction at the same location in the source code,
                        // targeting the same address register has already occurred
                        it->off.push_back(off);
                    }
                    else
                    {
                        // a global load instruction at the same location in the source code,
                        // targeting a different address register has already occurred,
                        // remember the encountered global load instruction
                        ld ld_obj;
                        ld_obj.type = "global_load";
                        ld_obj.off.push_back(off);
                        ld_obj.vaddr_srsrc = vaddr;
                        ld_obj.size = size;
                        ld_obj.PC_offset = match[match.size() - 2].str();
                        ld_obj.loc = loc_obj;
                        ld_vec.push_back(ld_obj);
                    }
                }
                else
                {
                    // no global load instruction has been seen in this code location yet,
                    // remember the encountered global load instruction
                    ld ld_obj;
                    ld_obj.type = "global_load";
                    ld_obj.off.push_back(off);
                    ld_obj.vaddr_srsrc = vaddr;
                    ld_obj.size = size;
                    ld_obj.PC_offset = match[match.size() - 2].str();
                    ld_obj.loc = loc_obj;
                    ld_vec.push_back(ld_obj);
                }
            }

            ld_cnt_map[krn_name] = ld_cnt;
            ld_map[krn_name] = ld_vec;
        }
        file.close();
    }
    else
    {
        std::cout << "==== ERROR" << std::endl;
        std::cout << "==== could not open the file " << filename << std::endl;
    }

    return std::make_tuple(ld_cnt_map, ld_map);
}

#endif // PARSER_AMDGCN_VECTORIZED_HPP