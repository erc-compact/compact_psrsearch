#pragma once
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include "data/search_mode_file.hpp"
namespace IO {
    class MultiTimeSeries {
        private:
            std::vector<float> dmList;
            std::vector<std::shared_ptr<SearchModeFile>> outFiles;
            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> dedispersedData;
            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> fullDedispersedData;

            std::size_t totalNSamples; // this is set to all samples or the gulped samples based on whether we are writing to file or not
            std::size_t dmListSize;
            std::string outDir;
            std::string outPrefix;
            std::string outSuffix;
            bool shouldWriteToFile;

        public:

            
            MultiTimeSeries(std::vector<float> dmList, std::string outDir, std::string outPrefix, std::string outSuffix, std::string outputFormat);
            MultiTimeSeries(std::vector<float> dmList, std::size_t gulpNSamples, std::size_t totalNSamples);
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