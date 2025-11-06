#ifndef PARSER_AMDGCN_ATOMIC_INSTRUCTION_HPP
#define PARSER_AMDGCN_ATOMIC_INSTRUCTION_HPP

#include "amdgcn_instructions.hpp"

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <regex>
#include <set>

/// @brief the structure contains information about the source code location of instructions in the assembly
struct location
{
    int line_num;
    std::string file_name;

    bool operator<(const location& loc) const
    {
        return line_num < loc.line_num;
    }
};

struct shared_atom
{
    std::string brc;
    std::string PC_offset;
    location loc;

    bool operator<(const shared_atom& shr_atom) const {
        return PC_offset < shr_atom.PC_offset;
    }
};

struct global_atom
{
    std::string brc;
    std::string PC_offset;
    location loc;

    bool operator<(const global_atom& gbl_atom) const {
        return PC_offset < gbl_atom.PC_offset;
    }
};

/// @brief structure that stores the number of global and shared atomic instruction occurrences, along with the
///        corresponding source code line numbers for each occurrence
struct atom
{
    int num_g;                   // number of global atomic instructions
    int num_s;                   // number of shared atomic instructions
    std::set<global_atom> gbl_atom; // set of source code line numbers corresponding to global atomic instructions
    std::set<shared_atom> shr_atom; // set of source code line numbers corresponding to shared atomic instructions
};

struct brc
{
    bool loop = false;     // is branch instruction more than one line after the branch label it references
    std::string tgt;       // target branch label of the branch instruction
    std::string PC_offset; // PC offset of the branch instruction
};

/// @brief          find global and shared atomic instructions in the assembly code
/// @param filename assembly file
/// @return         tuple of two maps:
///                 - first map holds occurences of global and shared atomic instructions per kernel
///                 - second map holds occurences of branch labels per kernel
inline std::tuple<std::unordered_map<std::string, atom>, std::unordered_map<std::string, std::vector<brc>>>
parser_atomic_instruction(const std::string &filename)
{
    std::fstream file(filename, std::ios::in);
    std::string line;

    std::unordered_map<std::string, atom> atom_map;
    std::unordered_map<std::string, std::vector<brc>> brc_map;
    std::vector<std::string> lbl_vec;

    std::vector<brc> brc_vec;

    std::string krn_name; // kernel name
    std::string cur_brc;  // current branch
    location loc_obj;          // line number in source code corresponding to the current assembly code line
    atom atom_obj;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, regex_krn_name))
            {
                atom_obj.num_g = 0;
                atom_obj.num_s = 0;
                atom_obj.gbl_atom.clear();
                atom_obj.shr_atom.clear();

                brc_vec.clear();

                krn_name = match[1].str();
            }

            if (std::regex_search(line, match, regex_loc))
            {
                loc_obj.line_num = std::stoi(match[2].str());
                loc_obj.file_name = match[1].str();
            }

            /** If the current line contains a global atomic instruction, increment the count of observed global atomic
             * instructions and record the line number of the newly detected global atomic instruction.
             *
             * If a branch instruction exists that targets the current branch label, record the line number of the newly
             * detected global atomic instruction.
            */
            if (std::regex_search(line, match, regex_FLAT("((?:global_atomic)\\w*)")) ||
                std::regex_search(line, match, regex_MIMG("((?:image_atomic)\\w*)", "([\\w\\[\\]:\\-]+)")) ||
                std::regex_search(line, match, regex_MUBUF("((?:buffer_atomic)\\w*)", "([\\w\\[\\]:\\-]+)")))
            {
                global_atom gbl_atom_obj;
                gbl_atom_obj.brc = cur_brc;
                gbl_atom_obj.PC_offset = match[match.size() - 2];
                gbl_atom_obj.loc = loc_obj;

                atom_obj.num_g++;
                atom_obj.gbl_atom.insert(gbl_atom_obj);
            }

            /** 
             * If the current line contains a shared atomic instruction, increment the count of observed shared atomic 
             * instructions and record the line number of the newly detected shared atomic instruction.
             *
             * If a branch instruction exists that targets the current branch label, record the line number of the newly 
             * detected shared atomic instruction.
            */
            if (std::regex_search(line, match, regex_DS("((?:ds_add|ds_and|ds_dec|ds_inc|ds_max|ds_min|ds_or|ds_rsub"
                                                                   "|ds_xor)\\w*)")))
            {
                shared_atom shr_atom_obj;
                shr_atom_obj.brc = cur_brc;
                shr_atom_obj.PC_offset = match[match.size() - 2];
                shr_atom_obj.loc = loc_obj;

                atom_obj.num_s++;
                atom_obj.shr_atom.insert(shr_atom_obj);
            }

            // if the current line contains a branch label, add it to the label vector
            if (std::regex_search(line, match, regex_brc_lbl))
            {
                cur_brc = match[1].str();
                lbl_vec.push_back(match[1].str());
            }

            /**
             * Check if the current line contains a branch instruction. If it does, determine whether it references a 
             * previously encountered branch label. If a reference to a branch label is found, add the location of its 
             * first instruction to the corresponding branch label structure. Additionally, if there are instructions 
             * between the current branch instruction and the target label, set the loop flag of the referenced label to
             * true, indicating that the branch forms a loop.
            */
            if (std::regex_search(line, match, regex_SOPP("((?:s_branch|s_cbranch)\\w*)")))
            {
                brc brc_obj;

                brc_obj.loop = false;
                brc_obj.tgt = match[2].str();
                brc_obj.PC_offset = match[match.size() - 2];

                if (std::find(lbl_vec.begin(), lbl_vec.end(), brc_obj.tgt) != lbl_vec.end())
                {
                    brc_obj.loop = true;
                }

                brc_vec.push_back(brc_obj);
            }

            brc_map[krn_name] = brc_vec;
            atom_map[krn_name] = atom_obj;
        }
        file.close();
    }
    else
    {
        std::cout << "==== ERROR" << std::endl;
        std::cout << "==== could not open the file " << filename << std::endl;
    }

    return std::make_tuple(atom_map, brc_map);
}

#endif // PARSER_AMDGCN_ATOMIC_INSTRUCTION_HPP
