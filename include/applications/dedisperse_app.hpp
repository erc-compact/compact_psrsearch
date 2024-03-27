/**
 * @file dedisperse_app.hpp
 * @brief Contains the declaration of the DedisperseCommandArgs class.
 */

#include <tclap/CmdLine.h>
#include <string>
#include <sstream>
#include <iostream>
#include "exceptions.hpp"
#include "data/search_mode_file.hpp"
#include "applications/common_arguments.hpp"
#include <typeinfo>

/**
 * @class DedisperseCommandArgs
 * @brief Represents the command line arguments for the dedisperse application.
 * 
 * This class inherits from APP::DataFileReadArgs, APP::FileWriteArgs, and APP::CommonArgs.
 * It provides additional arguments specific to the dedisperse application.
 */
class DedisperseCommandArgs: public APP::DataFileReadArgs, public APP::FileWriteArgs, public APP::CommonArgs
{
    public:
        float dmStart; /**< The first DM to dedisperse to. */
        float dmEnd; /**< The last DM to dedisperse to. */
        float dmTol; /**< The DM smearing tolerance. */
        float dmPulseWidth; /**< The pulse width. */

        size_t dedispGulp; /**< The number of samples to dedisperse at a time. */
        bool gulping; /**< Flag indicating if gulping is enabled. */

        std::string dmFile; /**< The file with a list of DMs to dedisperse to. */
        bool verbose; /**< Flag indicating if verbose mode is enabled. */

        int barycentre; /**< Flag indicating if barycentring the data before dedispersion is enabled. */
        int numGpus; /**< The number of GPUs to use for dedispersion. */
        int ramLimitGB; /**< The maximum amount of data to load into host RAM at a time (in GB). */

        TCLAP::ValueArg<float> argDmStart{"", "dm_start", "First DM to dedisperse to. (default =0)",false, 0.0, "float"};
        TCLAP::ValueArg<float> argDmEnd{"", "dm_end","Last DM to dedisperse to. (default = 2000)",false, 2000.0, "float"};
        TCLAP::ValueArg<float> argDmTol{"", "dm_tol","DM smearing tolerance (default=1.25)",false, 1.25, "float"};
        TCLAP::ValueArg<std::string> argDmFile{"", "dm_file","File with list of DMs to dedisperse to",false, "", "float"};
        TCLAP::ValueArg<size_t> argDedispGulp{"", "dedisp_gulp","Number of samples to dedisperse at a time",false, 0, "size_t"};
        TCLAP::SwitchArg argBarycentre{"", "barycentre", "Barycentre the data before dedispersion"};
        TCLAP::ValueArg<int> argNumGpus{"", "num_gpus", "Number of GPUs to use for dedispersion",false, 1, "int"};
        TCLAP::ValueArg<int> argRamLimitGB{"", "", "Maximum amount of data to load into host RAM at a time (in GB)",false, 0, "int"};

        /**
         * @brief Constructs a DedisperseCommandArgs object with default values.
         */
        DedisperseCommandArgs():dmStart(0.0), 
                                dmEnd(2000.0), 
                                dmTol(1.25), 
                                dmPulseWidth(0.0), 
                                dedispGulp(0),  
                                dmFile(""), 
                                barycentre(0),
                                numGpus(1),
                                ramLimitGB(0)
        {
            ArgsBase::registerParser(typeid(*this).name(), std::bind(&DedisperseCommandArgs::parse, this, std::placeholders::_1, std::placeholders::_2));
            ArgsBase::cmd.add(argDmStart);
            ArgsBase::cmd.add(argDmEnd);
            ArgsBase::cmd.add(argDmTol);
            ArgsBase::cmd.add(argDmFile);
            ArgsBase::cmd.add(argDedispGulp);
            ArgsBase::cmd.add(argBarycentre);
            ArgsBase::cmd.add(argNumGpus);
            ArgsBase::cmd.add(argRamLimitGB);
        }
        
        /**
         * @brief Parses the command line arguments.
         * 
         * @param argc The number of command line arguments.
         * @param argv The array of command line arguments.
         * 
         * @throws CustomException if both dm_tol and dm_file are set.
         */
        inline void parse(int argc, char **argv) override{
            if(argDmTol.isSet() && argDmFile.isSet())  {
                throw CustomException("You cannot set both dm_tol and dm_file");
            }
            dmStart = argDmStart.getValue();
            dmEnd = argDmEnd.getValue();
            dmTol = argDmTol.getValue();
            dmFile = argDmFile.getValue();
            gulping = argDedispGulp.isSet();
            dedispGulp = argDedispGulp.getValue();
            barycentre = argBarycentre.getValue();
            numGpus = argNumGpus.getValue();
            ramLimitGB = argRamLimitGB.getValue();
        }
};
