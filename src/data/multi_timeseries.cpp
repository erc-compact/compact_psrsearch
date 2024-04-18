#include "data/multi_timeseries.hpp"

using namespace IO;
MultiTimeSeries::MultiTimeSeries(std::shared_ptr<std::vector<float>> dmList, std::size_t gulpNSamples, bool shouldWriteToFile): MultiTimeSeries(dmList, gulpNSamples, gulpNSamples, shouldWriteToFile){}
MultiTimeSeries::MultiTimeSeries(std::shared_ptr<std::vector<float>> dmList, 
                                 std::size_t gulpNSamples, 
                                 std::size_t totalNSamples, 
                                 bool shouldWriteToFile){
this->dmList = dmList;
this->dmListSize = dmList->size();
this->gulpNSamples = gulpNSamples;
this->totalNSamples = totalNSamples;
this->shouldWriteToFile = shouldWriteToFile;
this->dedispersedData = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(gulpNSamples * dmListSize);

if(totalNSamples > gulpNSamples && !shouldWriteToFile) {
    this->fullDedispersedData = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(totalNSamples * dmListSize);
}

}


void MultiTimeSeries::initOutputOptions(std::string outDir, std::string outPrefix, std::string outSuffix, std::string outputFormat, std::shared_ptr<IO::SearchModeFile> searchModeFile){
    this->outDir = outDir;
    this->outPrefix = outPrefix;
    this->outSuffix = outSuffix;
    this->outFiles.reserve(dmListSize);
    for (std::size_t i = 0; i < dmListSize; i++){
        std::stringstream outFileName;

        if (!outDir.empty()) outFileName << outDir << "/";
        if (!outPrefix.empty()) outFileName << outPrefix << "_";

        outFileName << "DM" << std::setprecision(3) << std::fixed << this->dmList->at(i);
        if(!outSuffix.empty()) outFileName << "_" << outSuffix;
        outFileName << "." << SearchModeFile::getExtension(outputFormat);


        std::shared_ptr<SearchModeFile> outFile = SearchModeFile::createInstance(outFileName.str(), WRITE, outputFormat);
        outFile->copyHeaderFrom(searchModeFile);
        outFile->updateHeaderValue<float>(REFDM, this->dmList->at(i));
        
        outFile->writeHeader();
        outFile->openDataFile();
        outFiles.push_back(outFile);

       
    }
}

std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> MultiTimeSeries::getCurrentDedispersedDataPtr() {
    return this->dedispersedData;
}

void MultiTimeSeries::flush(){
    if (this->shouldWriteToFile){
        this->writeToFile();
    }
    else {
        fullDedispersedData->insert(fullDedispersedData->end(), std::make_move_iterator(dedispersedData->begin()), std::make_move_iterator(dedispersedData->end()));   
    }
    dedispersedData->clear();
}

void MultiTimeSeries::writeToFile(){
    for (std::size_t i = 0; i < dmListSize; i++){
        std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> iDedisp = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(dedispersedData->begin() + i * gulpNSamples, dedispersedData->begin() + (i + 1) * gulpNSamples);
        outFiles[i]->writeNBytes<DEDISP_OUTPUT_TYPE>(nSamplesWritten,  iDedisp);
    }
    this->nSamplesWritten += this->gulpNSamples;
}