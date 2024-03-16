#include "applications/dedisperse_app.hpp"
#include "data/search_mode_file.hpp"
#include "utils/gen_utils.hpp"
#include "operations/dedisperse.hpp"
#include "data/sigproc_file.hpp"
#include "data/presto_timeseries.hpp"
#include "utils/app_utils.hpp"
#include "exceptions.hpp"
#include "data/multi_timeseries.hpp"
#include "applications/common_arguments.hpp"
#include "TCLAP/CmdLine.h"
#include "data/Dedisperser.hpp"
#include "data/DedisperseCommandArgs.hpp"
#include "data/SearchModeFile.hpp"
#include "utils/generateListFromAsciiMaskFile.hpp"
#include "exceptions/CustomException.hpp"
#include <vector>
#include <cassert>
#include <memory>
#include <iostream>
#include <filesystem>

TCLAP::CmdLine ArgsBase::cmd("dedisperse", ' ', "0.1");


int main(int argc, char ** argv){
 

    DedisperseCommandArgs args;
    args.parseAll(argc, argv);  
    
    IO::SearchModeFile* searchModeFile = IO::SearchModeFile::createInstance(args.inputFile, READ, args.inputFormat);

    std::vector<float> fullDmList = (!args.dmFile.empty() && flleExists(args.dmFile)) ? OPS::Dedisperser.generateDmList(args.dmFile): 
                                               OPS::Dedisperser.generateDmList(args.dmStart, args.dmEnd, args.dmPulseWidth, args.dmTol);

    OPS::Dedisperser dedisperser(searchModeFile, args.numGpus, fullDmList, 
                            args.outputDir, args.outputPrefix, args.outputSuffix, args.outputFormat);




    int nChans = searchModeFile->getNchans();

    std::vector<DEDISP_BOOL> killmask = args.killmaskFile ? generateListFromAsciiMaskFile<DEDISP_BOOL>(args.killmaskFile, nChans) : 
                                                            std::vector<DEDISP_BOOL>(nChans, 1);    

    std::size_t nBytesToRead = 0;
    std::size_t startByte = 0;

    switch(args.selectionUnits){
        case BYTES:
            nBytesToRead = args.nbytes;
            startByte = args.startByte;
            break;
        case SECONDS:
            nBytesToRead = searchModeFile->timeToBytes(args.nsecs);
            startByte = searchModeFile->timeToBytes(args.startSec);
            break;
        case SAMPLES:
            nBytesToRead = searchModeFile->samplesToBytes(args.nSamps);
            startByte = searchModeFile->samplesToBytes(args.startSample);
            break;
        case NULL_STR:
            nBytesToRead = searchModeFile->getTotalDataSize();
            startByte = 0;
            break;
            
    }

    assert(startByte + nBytesToRead <= searchModeFile->getTotalDataSize());


    if (args.killmaskFile) dedisperser.setKillMask(args.killmaskFile);
    std::size_t gulpSize = args.gulping? args.dedispGulp: nBytesToRead;

    if (gulpSize < 2 * dedisperser.getMaxDelaySamples()){
        throw CustomException("Gulp size is smaller than 2 *  maximum delay");
    }

    std::size_t nSamplesToRead = searchModeFile->bytesToSamples(nBytesToRead);
    std::size_t bytesRead = 0;

    while (bytesRead < nBytesToRead){
        std::size_t bytesToRead = std::min(nBytesToRead - bytesRead, gulpSize);
        dedisperser.dedisperse(startByte + bytesRead, bytesToRead);
        bytesRead += bytesToRead;     
        dedisperser.writeData();


        searchModeFile->clearBuffer();

        

    }

    
    




    // std::size_t nSamplesToRead = searchModeFile->bytesToSamples(nBytesToRead);

    // std::size_t sizeOfDedispOutput = nSamplesToRead * fullDmList.size() * sizeof(DEDISP_OUTPUT_TYPE);
    // std::size_t ramLimit = args.ramLimitGB * 1e9;

    // if (ramLimit < sizeOfDedispOutput){
    //     throw std::runtime_error("RAM limit is smaller than the size of the dedispersed data");
    // }
    




    

    SearchModeFile* out_data_obj = SearchModeFile::create_instance(args.input_file, WRITE, args.output_file_format);
    out_data_obj->write_data(out_data, search_file_obj.get_data_size());





    return 0; 




}