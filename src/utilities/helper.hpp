// Contains helper functions used in both the nvidia and amd version

#ifndef GPUSCOUT_HELPER_HPP
#define GPUSCOUT_HELPER_HPP

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

// https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
std::string get_demangled_kernel(std::string kernel_name, std::string utility = "cu++filt") {
    std::string command = utility + " " + kernel_name;
    std::string result;
    FILE* stream;
    const int max_buffer = 256;
    char buffer[max_buffer];

    stream = popen(command.c_str(), "r");

    if (stream) {
        while (!feof(stream)) {
            if (fgets(buffer, max_buffer, stream) != NULL)
                result.append(buffer);
        }
        pclose(stream);
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

#endif //GPUSCOUT_HELPER_HPP