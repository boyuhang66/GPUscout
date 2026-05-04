#ifndef PARSER_AMDGCN_DEADLOCK_DETECTION_HPP
#define PARSER_AMDGCN_DEADLOCK_DETECTION_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

std::unordered_map<std::string, bool>
parser_deadlock_detection(const std::string &filename)
{
    std::string line;
    std::fstream file(filename, std::ios::in);

    std::unordered_map<std::string, bool> deadlock_map;

    bool deadlock_potential = false;
    bool inside_cas, brc_in_cas, sync_in_cas = false;

    std::string krn_name;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                deadlock_potential = false;
                inside_cas = false;
                brc_in_cas = false;
                sync_in_cas = false;

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_FLAT("(?:flat|global)_atomic_cmpswap\\w*")) ||
                std::regex_search(line, match, regex_MIMG("image_atomic_cmpswap\\w*", "([\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MUBUF("buffer_atomic_cmpswap\\w*", "([\\w\\[\\]:\\-]+)")))
            {
                inside_cas = true;
            }

            if (std::regex_search(line, match, regex_SOPP("s_cbranch\\w*")) && (inside_cas))
            {
                brc_in_cas = true;
            }

            if (std::regex_search(line, match, regex_SOPP("s_barrier")) && (brc_in_cas))
            {
                sync_in_cas = true;
                deadlock_potential = true;
            }

            if (std::regex_search(line, match, regex_FLAT("(?:flat|global)_atomic_swap\\w*")) ||
                std::regex_search(line, match, regex_MIMG("image_atomic_swap\\w*", "([\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MUBUF("buffer_atomic_swap\\w*", "([\\w\\[\\]:\\-]+)")))
            {
                inside_cas = false;
            }

            if (line.find("ATOM.E.EXCH") != std::string::npos)
            {
                inside_cas = false;
            }

            deadlock_map[krn_name] = deadlock_potential;
        }
        file.close();
    }
    else
    {
        std::cout << "Error :: Could not open the file: " << filename << std::endl;
    }

    return deadlock_map;
}

#endif // PARSER_AMDGCN_DEADLOCK_DETECTION_HPP
