
#include <tclap/CmdLine.h>
#include <string>
#include <sstream>
#include <iostream>
#include "exceptions.hpp"
#include <vector>
#include <cstdarg>

namespace APP {

    class ArgsBase {

        static TCLAP::CmdLine cmd;   

        protected:
            std::vector<std::function<void()>> functionList; // list of functions in different classes that parse the whole commandline

        public:
            ArgsBase(){}
            ~ArgsBase(){}


        public:
            virtual void parse(int argc, char **argv){
                ArgsBase::cmd.parse(argc, argv);
            }

            void registerParser(std::function<void()> f) {
            // add if function is not already in the list
                if (std::find(functionList.begin(), functionList.end(), f) == functionList.end()) {
                functionList.push_back(f);
                }
            }

            void parseAll(int argc, char **argv) {
                try {

                    for (auto &f : functionList) {
                        f(argc, argv);
                    }

                }catch(TCLAP::ArgException &e) {
                    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
                    throw e;
                } catch(CustomException &e) {
                    std::cerr << "error: " << e.what() << std::endl;
                    throw e;
                }
            }

    };

    class CommonArgs: public ArgsBase {

        public: 
            std::string verboseLevel;
            bool progressBar; 
            TCLAP::SwitchArg argProgressBar("p", "progress_bar", "Enable progress bar for DM search");
            TCLAP::ValueArg argVerbose("v", "verbose", "Verbose level (default = WARN)", false, "info", "string");
            

            CommonArgs() : verbose(false), 
                           progressBar(false) {
                registerParser(std::bind(&CommonArgs::parse, this, std::placeholders::_1, std::placeholders::_2));
                ArgsBase::cmd.add(argVerbose);
                ArgsBase::cmd.add(argProgressBar);
            }

            void parse(int argc, char** argv) override {
                std::string = argVerbose.getValue();
                progressBar = argProgressBar.getValue();
            }
            bool isAnyArgSet(const std::vector<TCLAP::Arg*>& argGroup) {
                for (const auto& arg : argGroup) {
                    if (arg->isSet()) {
                        return true;
                    }
                }
                return false;
            }
            template<typename... Groups>
            void checkExclusivity(Groups... groups) {
                try {
                    std::vector<bool> groupStatuses = {isAnyArgSet(groups)...};
                    int trueCount = std::count(groupStatuses.begin(), groupStatuses.end(), true);
                    if (trueCount > 1) {
                        stringstream exception_msg;
                        exception_msg << "You can only use one of the following options: ";
                        for (const auto& group : groups) {
                            for (const auto& arg : group) {
                                exception_msg << arg->getName() << ", ";
                            }
                        }
                        exception_msg << "but you used more than one.";
                        throw CustomException(exception_msg.str());
                    }
                }      
                } catch (TCLAP::ArgException &e) {
                    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
                    throw e;
                } catch(CustomException &e) {
                    std::cerr << "error: " << e.what() << std::endl;
                    throw e;
                }
            }
            

    };

    class DataFileReadArgs : public ArgsBase {

        public:
            size_t startByte;
            size_t nbytes;

            TCLAP::ValueArg<size_t> argStartByte("", "start_byte", "Byte to start reading from",false, 0, "size_t");
            TCLAP::ValueArg<size_t> argNBytes("", "nbytes", "Number of bytes to read",false, 0, "size_t");


            size_t startSample;
            size_t nsamps;        

            TCLAP::ValueArg<size_t> argStartSample("", "start_sample", "Sample to start reading from",false, 0, "size_t");
            TCLAP::ValueArg<size_t> argNSamps("", "nsamps", "Number of samples to read",false, 0, "size_t");

            float startSec;
            float nsecs;

            TCLAP::ValueArg<float> argStartSec("", "start_sec", "Second to start reading from",false, 0, "float");
            TCLAP::ValueArg<float> argNSecs("", "nsecs", "Number of seconds to read",false, 0, "float");

            std::string inputFile;
            std::string inputFormat;

            TCLAP::ValueArg<std::string> argInputFile("i", "input_file","Input file name",true, "", "string");   
            TCLAP::ValueArg<std::string> argInputFormat("", "input_format","Input file format",false, "", "string");


            std::string selectionUnits;

            DataFileArgs(): startByte(0), 
                            nbytes(-1), 
                            startSample(0), 
                            nsamps(-1), 
                            startSec(0), 
                            nsecs(-1),
                            inputFile(NULL_STR),
                            inputFormat("sigproc_filterbank"), 
                             {

                registerParser(std::bind(&DataFileArgs::parse, this, std::placeholders::_1, std::placeholders::_2));

                ArgsBase::cmd.add(argStartByte);
                ArgsBase::cmd.add(argNBytes);

                ArgsBase::cmd.add(argStartSample);
                ArgsBase::cmd.add(argNSamps);

                ArgsBase::cmd.add(argStartSec);
                ArgsBase::cmd.add(argNSecs);

                ArgsBase::cmd.add(argInputFile);
                ArgsBase::cmd.add(argInputFormat);                


            }

            inline void parse(int argc, char **argv) override {
                
                checkExclusivity({argStartByte, argNBytes}, {argStartSample, argNSamps}, {argStartSec, argNSecs});

                startByte = argStartByte.getValue();
                nbytes = argNBytes.getValue();
                startSample = argStartSample.getValue();
                nsamps = argNSamps.getValue();
                startSec = argStartSec.getValue();
                nsecs = argNSecs.getValue();

                if (nbytes != -1) selectionUnits = BYTES;
                else if (nsamps != -1)selectionUnits = SAMPLES;                  
                else if (nsecs != -1) selectionUnits = SECONDS;    
                else selectionUnits = NULL_STR;  


                if(! arg_input_format.isSet()) {
                    // guess format based on file extension
                    std::string input_file = arg_input_file.getValue();
                    std::string ext = input_file.substr(input_file.find_last_of(".") + 1);
                    if(ext == "fil") input_format = "sigproc_filterbank";
                    else if(ext == "dat") input_format = "presto_timeseries";
                    else if(ext == "tim") input_format = "sigproc_timeseries";
                    else  throw CustomException("Could not guess input file format from extension. Please specify using -f option");
                
                }

            }

    };

    class FileWriteArgs: public ArgsBase {

        public:
            std::string outputDir;
            std::string outputFormat;
            std::string outputPrefix;
            std::string outputSuffix;
            std::string outputFileName;

            TCLAP::ValueArg<std::string> argOutputDir("o", "out_dir","Output directory",true, "./", "string");
            TCLAP::ValueArg<std::string> argOutputFormat("", "out_format","Output file format",false, "presto_timeseries", "string");
            TCLAP::ValueArg<std::string> argOutputPrefix("", "out_prefix","Prefix for output file names",false, "", "string");
            TCLAP::ValueArg<std::string> argOutputSuffix("", "out_suffix","Suffix for output file names",false, "", "string");
            TCLAP::ValueArg<std::string> argOutputFileName("", "outfile","Output file name",false, "", "string");

            DataFileWriteArgs(): outputDir("./"), 
                                 outputFormat("presto_timeseries"), 
                                 outputPrefix(""), 
                                 outputSuffix(""), 
                                 outputFileName("") {

                registerParser(std::bind(&DataFileWriteArgs::parse, this, std::placeholders::_1, std::placeholders::_2);

                ArgsBase::cmd.add(argOutputDir);
                ArgsBase::cmd.add(argOutputFormat);
                ArgsBase::cmd.add(argOutputPrefix);
                ArgsBase::cmd.add(argOutputSuffix);
                ArgsBase::cmd.add(argOutputFileName);

            }

            inline void parse(int argc, char **argv) override{

                if(argOutputFileName.isSet() && (argOutputPrefix.isSet() || argOutputSuffix.isSet())) {
                    throw CustomException("Cannot set both outfile and out_prefix/out_suffix");
                }
                
                outputDir = argOutputDir.getValue();
                outputFormat = argOutputFormat.getValue();
                outputPrefix = argOutputPrefix.getValue();
                outputSuffix = argOutputSuffix.getValue();
                outputFileName = argOutputFileName.getValue();

            }

    };



    
};
