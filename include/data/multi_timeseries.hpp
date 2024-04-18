#pragma once
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include "data/search_mode_file.hpp"
namespace IO {
    class MultiTimeSeries {
        private:
            std::shared_ptr<std::vector<float>> dmList;
            std::vector<std::shared_ptr<SearchModeFile>> outFiles;
            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> dedispersedData;
            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> fullDedispersedData;

            std::size_t totalNSamples; 
            std::size_t gulpNSamples;  
            std::size_t nSamplesWritten;          
            std::size_t dmListSize;
            std::string outDir;
            std::string outPrefix;
            std::string outSuffix;
            bool shouldWriteToFile;

        public:
            MultiTimeSeries(std::shared_ptr<std::vector<float>> dmList, std::size_t gulpNSamples, std::size_t totalNSamples, bool shouldWriteToFile);
            MultiTimeSeries(std::shared_ptr<std::vector<float>> dmList, std::size_t gulpNSamples, bool shouldWriteToFile);
            MultiTimeSeries(std::shared_ptr<std::vector<float>> dmList, std::size_t gulpNSamples, std::size_t totalNSamples);

            void initOutputOptions(std::string outDir, std::string outPrefix, std::string outSuffix, std::string outputFormat, std::shared_ptr<IO::SearchModeFile> searchModeFile);
            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> getCurrentDedispersedDataPtr();
            void flush();
            virtual ~MultiTimeSeries() = default;


        private:
            void writeToFile();

            // DEDISP_OUTPUT_TYPE& operator()(std::size_t iDm, std::size_t iSample){
            //     return this->dedispersedData.get()[iDm * this->dmListSize + iSample];
            // }
            // inline void writeToFile(){
            //     for (std::size_t i = 0; i < dmListSize; i++){
            //         outFiles[i]->writeNBytes(this->dedispersedData.get()[i], this->totalNSamples * sizeof(DEDISP_OUTPUT_TYPE));
            //     }
            // }

    };
};