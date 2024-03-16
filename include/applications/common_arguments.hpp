
#include <tclap/CmdLine.h>
#include <string>
#include <sstream>
#include <iostream>
#include "exceptions.hpp"
#include <vector>
#include <cstdarg>
#include <functional>

namespace APP {

    /**
     * @brief Base class for command line arguments. 
     * Contains static TCLAP command line instance, and a list of functions that parse the command line.
     * This list is populated by the registerParser function, that is called by every constructor of a class that inherits from ArgsBase.
     * For each argument, the following steps are needed:
     * 1. Create a member variable for the argument.
     * 2. Create a TCLAP::ValueArg or TCLAP::SwitchArg object for the argument.
     * 3. Add the object to the TCLAP command line instance.
     * 4. Add a parse function that sets the member variable to the value of the argument.
     * 5. Call the registerParser function with the parse function as argument.
     * 6. Call the parseAll function with the command line arguments.
     */

    class ArgsBase {

    public:
        static TCLAP::CmdLine cmd;

    protected:
    
        static std::vector<std::function<void(int, char **)>> functionList; // list of functions in different classes that parse the whole commandline

        /**
         * @brief Registers a parser function to be called when parsing the command line.
         * @param f The parser function to register.
         */
        static void registerParser(std::function<void(int, char **)> f) {
            // add if function is not already in the list
            if (std::find(functionList.begin(), functionList.end(), f) == functionList.end()) {
                functionList.push_back(f);
            }
        }

    public:
        ArgsBase() {}
        ~ArgsBase() {}

        /**
         * @brief Parses the command line arguments. This function is called by the parseAll function, and is overriden by the classes that inherit from ArgsBase.
         * @param argc The number of command line arguments.
         * @param argv The array of command line arguments.
         */
        virtual void parse(int argc, char **argv) {
            ArgsBase::cmd.parse(argc, argv);
        }

        /**
         * @brief Parses all registered command line arguments.
         * @param argc The number of command line arguments.
         * @param argv The array of command line arguments.
         */
        static void parseAll(int argc, char **argv) {
            try {
                for (auto &f : functionList) {
                    f(argc, argv);
                }
            } catch (TCLAP::ArgException &e) {
                std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
                throw e;
            } catch (CustomException &e) {
                std::cerr << "error: " << e.what() << std::endl;
                throw e;
            }
        }

         /**
         * @brief Checks the exclusivity of multiple argument groups.
         * @tparam Groups The argument groups to check.
         * @param groups The argument groups to check.
         */
        template <typename... Groups>
        void checkExclusivity(Groups... groups) {
            try {
                std::vector<bool> groupStatuses = {isAnyArgSet(groups)...};
                int trueCount = std::count(groupStatuses.begin(), groupStatuses.end(), true);
                if (trueCount > 1) {
                    std::stringstream exception_msg;
                    exception_msg << "You can only use one of the following options: ";

                    (auto &&... groups) {
                        (([&exception_msg](const auto &group) {
                            for (const auto &arg : group) {
                                exception_msg << arg->getName() << ", ";
                            }
                        }(groups)),
                         ...);
                    }(groups...);
                    exception_msg << "but you used more than one.";
                    throw CustomException(exception_msg.str());
                }
            } catch (TCLAP::ArgException &e) {
                std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
                throw e;
            } catch (CustomException &e) {
                std::cerr << "error: " << e.what() << std::endl;
                throw e;
            }
        }
    };

    /**
     * @brief Class for common command line arguments.
     */
    class CommonArgs : public ArgsBase {

    public:
        std::string verboseLevel;
        bool progressBar;
        TCLAP::SwitchArg argProgressBar{"p", "progress_bar", "Enable progress bar for DM search"};
        TCLAP::ValueArg<std::string> argVerbose{"v", "verbose", "Verbose level (default = WARN)", false, "info", "string"};

        /**
         * @brief Constructs a CommonArgs object.
         */
        CommonArgs() : verboseLevel("WARN"),
                       progressBar(false) {
            ArgsBase::registerParser(std::bind(&CommonArgs::parse, this, std::placeholders::_1, std::placeholders::_2));
            ArgsBase::cmd.add(argVerbose);
            ArgsBase::cmd.add(argProgressBar);
        }

        /**
         * @brief Parses the command line arguments.
         * @param argc The number of command line arguments.
         * @param argv The array of command line arguments.
         */
        void parse(int argc, char **argv) override {
            verboseLevel = argVerbose.getValue();
            progressBar = argProgressBar.getValue();
        }

        /**
         * @brief Checks if any argument in the given group is set.
         * @param argGroup The group of arguments to check.
         * @return True if any argument in the group is set, false otherwise.
         */
        bool isAnyArgSet(const std::vector<TCLAP::Arg *> &argGroup) {
            for (const auto &arg : argGroup) {
                if (arg->isSet()) {
                    return true;
                }
            }
            return false;
        }

       
    };

    /**
     * @brief Class for command line arguments related to reading data files.
     */
    class DataFileReadArgs : public ArgsBase {

    public:
        std::size_t startByte;
        std::size_t nBytes;

        TCLAP::ValueArg<std::size_t> argStartByte{"", "start_byte", "Byte to start reading from", false, 0, "std::size_t"};
        TCLAP::ValueArg<std::size_t> argNBytes{"", "nbytes", "Number of bytes to read", false, 0, "std::size_t"};

        std::size_t startSample;
        std::size_t nSamps;

        TCLAP::ValueArg<std::size_t> argStartSample{"", "start_sample", "Sample to start reading from", false, 0, "std::size_t"};
        TCLAP::ValueArg<std::size_t> argNSamps{"", "nSamps", "Number of samples to read", false, 0, "std::size_t"};

        float startSec;
        float nSecs;

        TCLAP::ValueArg<float> argStartSec{"", "start_sec", "Second to start reading from", false, 0, "float"};
        TCLAP::ValueArg<float> argNSecs{"", "nsecs", "Number of seconds to read", false, 0, "float"};

        std::string inputFile;
        std::string inputFormat;

        TCLAP::ValueArg<std::string> argInputFile{"i", "input_file", "Input file name", true, "", "string"};
        TCLAP::ValueArg<std::string> argInputFormat{"", "input_format", "Input file format", false, "", "string"};

        std::string selectionUnits;

        std::string killFile;
        std::string birdiesFile;

        TCLAP::ValueArg<std::string> argKillFile{"", "kill_file", "File with list of channels to kill", false, "", "string"};
        TCLAP::ValueArg<std::string> argBirdiesFile{"", "birdies_file", "File with list of frequencies to kill", false, "", "string"};

        /**
         * @brief Constructs a DataFileReadArgs object.
         */
        DataFileReadArgs() : startByte(0),
                             nBytes(-1),
                             startSample(0),
                             nSamps(-1),
                             startSec(0),
                             nSecs(-1),
                             inputFile(NULL_STR),
                             inputFormat("sigproc_filterbank"),
                             selectionUnits(NULL_STR),
                             killFile(NULL_STR),
                             birdiesFile(NULL_STR) {

            ArgsBase::registerParser(std::bind(&DataFileReadArgs::parse, this, std::placeholders::_1, std::placeholders::_2));

            ArgsBase::cmd.add(argStartByte);
            ArgsBase::cmd.add(argNBytes);

            ArgsBase::cmd.add(argStartSample);
            ArgsBase::cmd.add(argNSamps);

            ArgsBase::cmd.add(argStartSec);
            ArgsBase::cmd.add(argNSecs);

            ArgsBase::cmd.add(argInputFile);
            ArgsBase::cmd.add(argInputFormat);

            ArgsBase::cmd.add(argKillFile);
            ArgsBase::cmd.add(argBirdiesFile);
        }

        /**
         * @brief Parses the command line arguments.
         * @param argc The number of command line arguments.
         * @param argv The array of command line arguments.
         */
        inline void parse(int argc, char **argv) override {

            checkExclusivity(std::vector<TCLAP::Arg*>{&argStartByte, &argNBytes}, 
                            std::vector<TCLAP::Arg*>{&argStartSample, &argNSamps}, 
                            std::vector<TCLAP::Arg*>{&argStartSec, &argNSecs});
            startByte = argStartByte.getValue();
            nBytes = argNBytes.getValue();
            startSample = argStartSample.getValue();
            nSamps = argNSamps.getValue();
            startSec = argStartSec.getValue();
            nSecs = argNSecs.getValue();

            inputFile = argInputFile.getValue();
            inputFormat = argInputFormat.getValue();
            killFile = argKillFile.getValue();
            birdiesFile = argBirdiesFile.getValue();

            if (nBytes > 0)
                selectionUnits = BYTES;
            else if (nSamps > 0)
                selectionUnits = SAMPLES;
            else if (nSecs > 0)
                selectionUnits = SECONDS;
            else
                selectionUnits = NULL_STR;

            if (!argInputFormat.isSet()) {
                // guess format based on file extension
                std::string input_file = argInputFile.getValue();
                std::string ext = inputFile.substr(inputFile.find_last_of(".") + 1);
                if (ext == "fil")
                    inputFormat = "sigproc_filterbank";
                else if (ext == "dat")
                    inputFormat = "presto_timeseries";
                else if (ext == "tim")
                    inputFormat = "sigproc_timeseries";
                else
                    throw CustomException("Could not guess input file format from extension. Please specify using -f option");
            }
        }
    };

    /**
     * @brief Class for command line arguments related to writing output files.
     */
    class FileWriteArgs : public ArgsBase {

    public:
        std::string outputDir;
        std::string outputFormat;
        std::string outputPrefix;
        std::string outputSuffix;

        TCLAP::ValueArg<std::string> argOutputDir{"o", "out_dir", "Output directory", true, "./", "string"};
        TCLAP::ValueArg<std::string> argOutputFormat{"", "out_format", "Output file format", false, "presto_timeseries", "string"};
        TCLAP::ValueArg<std::string> argOutputPrefix{"", "out_prefix", "Prefix for output file names", false, "", "string"};
        TCLAP::ValueArg<std::string> argOutputSuffix{"", "out_suffix", "Suffix for output file names", false, "", "string"};

        /**
         * @brief Constructs a FileWriteArgs object.
         */
        FileWriteArgs() : outputDir("./"),
                         outputFormat("presto_timeseries"),
                         outputPrefix(""),
                         outputSuffix("") {
            ArgsBase::registerParser(std::bind(&FileWriteArgs::parse, this, std::placeholders::_1, std::placeholders::_2));
            ArgsBase::cmd.add(argOutputDir);
            ArgsBase::cmd.add(argOutputFormat);
            ArgsBase::cmd.add(argOutputPrefix);
            ArgsBase::cmd.add(argOutputSuffix);
        }


        void parse(int argc, char **argv) override{
        
            outputDir = argOutputDir.getValue();
            outputFormat = argOutputFormat.getValue();
            outputPrefix = argOutputPrefix.getValue();
            outputSuffix = argOutputSuffix.getValue();

        }

    };



    
};
