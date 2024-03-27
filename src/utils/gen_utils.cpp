#include "utils/gen_utils.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <system_error>
#include <cerrno>
#include <cassert>

int fileOpen(FILE **file, const std::string absolutename, const std::string mode) {
    *file = fopen(absolutename.c_str(), mode.c_str());

    if (!*file)
    {
        std::error_code ec(errno, std::generic_category());
        std::cerr << "could not open '" << absolutename << "' in mode '" << mode << "' Error: " << ec.message() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


bool fileExists(const std::string name) {

    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

int checkSize(unsigned long req, unsigned long got) {
    if (req == got)
        return EXIT_SUCCESS;
    else
    {
        std::cerr << " Could not read " << req << " number of bytes. got only " << got << "bytes." << std::endl;
        return EXIT_FAILURE;
    }
}

std::string removeWhiteSpace(const std::string& str) {
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}


bool caseInsensitiveCompare(const std::string& s1, const std::string& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }

    return std::equal(s1.begin(), s1.end(), s2.begin(),
        [](char c1, char c2) {
            return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
        }
    );
}

std::string getExtension(const std::string& fileName) {
    std::string::size_type idx;
    idx = fileName.rfind('.');

    if (idx != std::string::npos)
        return fileName.substr(idx + 1);
    else
        return "";
}

std::string replaceExtension(const std::string& fileName, const std::string& newExtension) {
    std::string::size_type idx;
    idx = fileName.rfind('.');

    if (idx != std::string::npos)
        return fileName.substr(0, idx) + "." + newExtension;
    else
        return fileName + "." + newExtension;
}




