#include "applications/dedisperse_app.hpp"
#include "data/search_mode_file.hpp"
#include "utils/gen_utils.hpp"
#include "operations/dedisperse.hpp"
#include "data/sigproc_filterbank.hpp"
#include "data/presto_timeseries.hpp"
#include "utils/app_utils.hpp"
#include "exceptions.hpp"
#include "data/multi_timeseries.hpp"
#include "applications/common_arguments.hpp"
#include "tclap/CmdLine.h"
#include <vector>
#include <cassert>
#include <memory>
#include <iostream>

TCLAP::CmdLine APP::ArgsBase::cmd("dedisperse", ' ', "0.1");



int main(int argc, char ** argv){

 

    DedisperseCommandArgs args;
    APP::ArgsBase::parseAll(argc, argv); 
     

    std::shared_ptr<IO::SearchModeFile> searchModeFile = IO::SearchModeFile::createInstance(args.inputFile, READ, args.inputFormat);



    std::shared_ptr<std::vector<float>> fullDmList = std::make_shared<std::vector<float>>();
     if(!args.dmFile.empty() && fileExists(args.dmFile)) OPS::Dedisperser::populateDMList(fullDmList, args.dmFile);
     else OPS::Dedisperser::populateDMList(fullDmList, args.dmStart, args.dmEnd, args.dmPulseWidth, args.dmTol, 
                            searchModeFile->getValueForKey<float>(TSAMP),searchModeFile->getValueForKey<float>(FCH1),
                            searchModeFile->getValueForKey<float>(FOFF), searchModeFile->getValueForKey<int>(NCHANS));


    std::unique_ptr<OPS::Dedisperser> dedisperser; 

    dedisperser = std::make_unique<OPS::Dedisperser>(searchModeFile, args.numGpus, fullDmList, true, args.dedispGulp);
    std::string dataFilePrefix = args.inputFile.substr(0, args.inputFile.find_last_of("."));
    if(args.outputPrefix.empty()) args.outputPrefix = dataFilePrefix;


    dedisperser->setOutputOptions(args.outputDir, args.outputPrefix, args.outputSuffix, args.outputFormat, searchModeFile);


    int nChans = searchModeFile->getNChans();


    std::shared_ptr<std::vector<DEDISP_BOOL>> killmask = !args.killFile.empty() ? 
                                            generateListFromAsciiMaskFile<DEDISP_BOOL>(args.killFile, nChans) : 
                                            std::make_shared<std::vector<DEDISP_BOOL>>(nChans, 1);    


    std::size_t nBytesToRead = 0;
    std::size_t startByte = 0;

    if(args.selectionUnits == BYTES) {
        nBytesToRead = args.nBytes;
        startByte = args.startByte;

    } else if(args.selectionUnits == SECONDS) {
        nBytesToRead = searchModeFile->timeToBytes(args.nSecs);
        startByte = searchModeFile->timeToBytes(args.startSec);

    } else if(args.selectionUnits == SAMPLES) {
        nBytesToRead = searchModeFile->samplesToBytes(args.nSamps);
        startByte = searchModeFile->samplesToBytes(args.startSample);

    } else if(args.selectionUnits == NULL_STR) {
        nBytesToRead = searchModeFile->getTotalDataSize();
        startByte = 0;
    }

    assert(startByte + nBytesToRead <= searchModeFile->getTotalDataSize());


    if (!args.killFile.empty()) dedisperser->setKillMask(args.killFile);
    std::size_t gulpSize = args.gulping? args.dedispGulp: nBytesToRead;



    if (gulpSize < 2 * dedisperser->getMaxDelaySamples()){
        throw InvalidInputs("Gulp size is smaller than 2 *  maximum delay");
    }

    std::size_t nSamplesToRead = searchModeFile->bytesToSamples(nBytesToRead);
    std::size_t bytesRead = 0;

    while (bytesRead < nBytesToRead){


        std::size_t bytesToRead = std::min(nBytesToRead - bytesRead, gulpSize);


        dedisperser->dedisperse(startByte + bytesRead, bytesToRead);
        bytesRead += bytesToRead;     

        searchModeFile->clearBuffer();

        

    }

    
    




    // std::size_t nSamplesToRead = searchModeFile->bytesToSamples(nBytesToRead);

    // std::size_t sizeOfDedispOutput = nSamplesToRead * fullDmList.size() * sizeof(DEDISP_OUTPUT_TYPE);
    // std::size_t ramLimit = args.ramLimitGB * 1e9;

    // if (ramLimit < sizeOfDedispOutput){
    //     throw std::runtime_error("RAM limit is smaller than the size of the dedispersed data");
    // }
    




    

    // SearchModeFile* out_data_obj = SearchModeFile::create_instance(args.input_file, WRITE, args.output_file_format);
    // out_data_obj->write_data(out_data, search_file_obj.get_data_size());





    return 0; 




}