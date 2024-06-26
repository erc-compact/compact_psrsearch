#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>





/**
 * Standard range file has one number per line
 * If there is a colon, then it is of the format min:max:step. 
 * If step is missing, it is assumed to be 1.0  
 * 
*/

template <typename T> 
std::shared_ptr<std::vector<T>> generateListFromAsciiRangeFile(const std::string ifile) {
    std::ifstream infile;
    std::string str;
    infile.open(ifile.c_str(), std::ifstream::in);
    ErrorChecker::checkFileError(infile, ifile);
    std::shared_ptr<std::vector<T>>  outList = std::make_shared<std::vector<T>>();

    while (!infile.eof())
    {
        std::getline(infile, str);
        std::stringstream ss(str);
        std::string token;
        std::vector<std::string> tokens;
        // if : not in string, directly convert and add number
        // if : in string, split and add range
        if (str.find(':') == std::string::npos)
            outList->push_back(std::atof(str.c_str()));

        else
        {
            while (std::getline(ss, token, ':'))
                tokens.push_back(token);

            assert(tokens.size() > 1 && tokens.size() <= 3);

            float min = std::atof(tokens[0].c_str());
            float max = std::atof(tokens[1].c_str());
            float step = tokens.size() == 3 ? std::atof(tokens[2].c_str()) : 1.0;

            for (float k = min; k <= max; k += step)
                outList->push_back(k);
        }
    }
    return outList;
}
/**
 * Standard mask file has 1 for good channels and 0 for bad channels
 * 
*/
template <typename T> 
std::shared_ptr<std::vector<T>> generateListFromAsciiMaskFile(const std::string ifile, int size) {
    std::ifstream infile;
    std::string str;
    infile.open(ifile.c_str(), std::ifstream::in);
    ErrorChecker::checkFileError(infile, ifile);

    std::shared_ptr<std::vector<T>>  outList = std::make_shared<std::vector<T>>(size, 1); // all are good channels to begin with. 

    std::vector<std::string> lines;
    while (!infile.eof())
    {
        std::getline(infile, str);
        lines.push_back(str);
    }

    if(lines.size() == size){
        for(int ii = 0; ii < size; ii++){
            outList->at(ii) = std::atoi(lines[ii].c_str());
        }
    }
    else {
        std::cout <<  "WARNING: number of lines not the same as input size, attempting to parse as a range mask. " << std::endl;
        for(int ii = 0; ii < lines.size(); ii++){

            if (!str.find(':')) {
                outList->at(std::atoi(lines[ii].c_str())) = 0; // if no range, set corresponding index to bad.
            }
            else {
                std::stringstream ss(lines[ii]);
                std::string token;
                std::vector<std::string> tokens;
                while (std::getline(ss, token, ':'))
                    tokens.push_back(token);

                assert(tokens.size() > 1 && tokens.size() <= 3);
                // range, parse it as min:max:step and set all values in the range to bad.

                int min = std::atoi(tokens[0].c_str());
                int max = std::atoi(tokens[1].c_str());
                int step = tokens.size() == 3 ? std::atof(tokens[2].c_str()) : 1;

                for (int k = min; k <= max; k+=step) outList->at(k) = 0;

            }
        }
    }

    return outList;


}
