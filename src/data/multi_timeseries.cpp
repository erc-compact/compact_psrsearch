#include "data/multi_timeseries.hpp"

using namespace IO;

MultiTimeSeries::MultiTimeSeries(std::vector<float> dmList, std::string outDir, std::string outPrefix, std::string outSuffix, std::string outputFormat)
{
    this->outDir = outDir;
    this->outPrefix = outPrefix;
    this->outSuffix = outSuffix;
    this->outFiles.resize(dmListSize);
    for (std::size_t i = 0; i < dmListSize; i++){
        std::stringstream outFileName;
        outFileName << outDir << "/" << outPrefix << "_DM" << std::setprecision(3) << std::fixed << dmList[i] << "_" << outSuffix;
        outFiles.push_back(SearchModeFile::createInstance(outFileName.str(), WRITE, outputFormat));
    }
}

MultiTimeSeries::MultiTimeSeries(std::vector<float> dmList, std::size_t gulpNSamples, std::size_t totalNSamples){
    this->dmList = dmList;
    this->dmListSize = dmList.size();
    this->gulpNSamples = gulpNSamples;
    this->totalNSamples = totalNSamples;
    this->dedispersedData = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(gulpNSamples * dmListSize);
    this->fullDedispersedData = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(totalNSamples * dmListSize);
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
        dedispersedData->clear();
    }
}

void MultiTimeSeries::writeToFile(){
    for (std::size_t i = 0; i < dmListSize; i++){
        std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> iDedisp = std::make_shared<std::vector<DEDISP_OUTPUT_TYPE>>(dedispersedData->begin() + i * gulpNSamples, dedispersedData->begin() + (i + 1) * gulpNSamples);
        outFiles[i]->writeNBytes<DEDISP_OUTPUT_TYPE>(nSamplesWritten,  iDedisp);
    }
    this->nSamplesWritten += this->gulpNSamples;
}