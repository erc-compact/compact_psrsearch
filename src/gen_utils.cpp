#include "gen_utils.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <system_error>

int file_open(FILE **file, const std::string absolutename, const std::string mode) {
    *file = fopen(absolutename.c_str(), mode.c_str());

    if (!*file)
    {
        std::error_code ec(errno, std::generic_category());
        std::cerr << "could not open '" << absolutename << "' in mode '" << mode << "' Error: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


bool file_exists(const std::string name) {

    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

int check_size(unsigned long req, unsigned long got) {
    if (req == got)
        return EXIT_SUCCESS;
    else
    {
        std::cerr << " Could not read " << req << " number of bytes. got only " << got << "bytes." << std::endl;
        return EXIT_FAILURE;
    }
}

std::string remove_whitespace(const std::string& str) {
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}