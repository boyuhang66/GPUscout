/**
 * @author Benedik Deike
 */

#ifndef AMDGCN_INSTRUCTIONS_HPP
#define AMDGCN_INSTRUCTIONS_HPP

#include <regex>

enum class RegexKind { NONE, FLAT, MIMG, MUBUF, MTBUF };

// matches lines containing a command with the demangled kernel name
// ; vectorAdd(float const*, float const*, float*, int)():
std::regex regex_krn_name_demangled("^;"
                          "\\s"  // matches leading whitespace
                          "(\\w+)" // matches kernel name (group 1)
                          "\\([\\w,\\*\\s]*\\)" // matches parameters
                          "\\(\\)" // matches ()
                          ":\\s*"); // matches : and following whitespaces

// matches lines containing a command with the mangled kernel name
// ; _Z14spillingKernelPfS_():
std::regex regex_krn_name("^;"
                          "\\s"  // matches leading whitespace
                          "(_+Z\\d+" // matches _Z<digits> (group 1)
                          "\\w+)" // matches kernel name  (group 1)
                          "\\(\\)" // matches ()
                          ":\\s*"); // matches : and following whitespaces

// matches lines containing source code location information
// ; /home/ab62cde2/SourceCode/DoubleKernel.cpp:6
std::regex regex_loc("^;\\s.*/([^/]+\\.(?:cpp|hpp|h)):(\\d+)\\s*");

// matches lines containing a branch label
// 0000000000001a94 <L0>:
std::regex regex_brc_lbl("^[0-9a-z]{16}\\s<(L[0-9][0-9]?)>:\\s*");

// instruction reference: https://llvm.org/docs/AMDGPU/AMDGPUAsmGFX90a.html
// Instruction encodings are used because the regular expressions are not as tight as they could/should be.

inline std::regex regex_MUBUF(const std::string& op, const std::string& srsrc)
{
    return std::regex(
        "^"                                       // regex must match from the beginning of the string
        "\\s*"                                    // matches any leading whitespace
        + op +
        "\\s"                                     // matches the whitespace following the instruction name
        "([\\w\\[\\]:\\-]+)"                      // captures the first operand 
        ",\\s"                                    // matches the comma and whitespace after an operand
        "([\\w\\[\\]:\\-]+)"                      // captures the second operand
        ",\\s"                                    // matches the comma and whitespace after an operand
        + srsrc +
        ",\\s"                                    // matches the comma and whitespace after an operand
        "([\\w\\[\\]:\\-]+)"                      // captures the fourth operand
        "[\\s\\w\\[\\]:\\-]*"                     // matches any additional parameters
        "\\s*"                                    // matches any whitespaces after the instruction
        "//\\s([A-F0-9]*):"                       // captures the memory location of the instruction
        "\\s"                                     // matches the whitespace after the memory location
        // "(E[0-3][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // captures the binary representation of the instruction
                                                    // instruction encoding begins with: 111000
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"                                      // matches any trailing whitespace
    );
}

inline std::regex regex_MTBUF(const std::string& op, const std::string& srsrc)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        + srsrc +
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(E[8|9|A|B][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 111010
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_MIMG(const std::string& op, const std::string &srsrc)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        + srsrc +
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(F[0-3][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 111100
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_DS(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "(?:\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(D[9|A|B][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 110110
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_FLAT(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(D[C-F][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 110111
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_VOP1(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 0111111
                                              // instruction observed don't seem to adhere to the opcode,
                                              // therefore opcode check is not present
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_VOP2(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "(?:,)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "([0-7][A-F0-9]{7}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 0
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_VOP3(const std::string &op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "([D][0-3][A-F0-9]{6}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 110100
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_VOP3P(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)"
        ",\\s"
        "([\\w\\[\\]:\\-]+)"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "(?:,\\s)?"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(D3[8-9A-F][A-F0-9]{5}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 1101001110
                                                        // observerd v_accvgpr_write instructions ignore the last 0 in the
                                                        // encoding, so the third encoding symbol goes to F instead of B
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

inline std::regex regex_SOPP(const std::string& op)
{
    return std::regex(
        "^"
        "\\s*"
        + op +
        "\\s"
        "([\\w\\[\\]:\\-]+)?"
        "[\\s\\w\\[\\]:\\-]*"
        "\\s*"
        "//\\s([A-F0-9]*):"
        "\\s"
        // "(BF[8-9A-F][A-F0-9]{5}(?:\\s[A-F0-9]{8})?)" // instruction encoding begins with: 101111111
        "([A-F0-9]{8}(?:\\s[A-F0-9]{8})?)"
        "\\s*"
    );
}

#endif // AMDGCN_INSTRUCTIONS_HPP