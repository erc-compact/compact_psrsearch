#pragma once
#include "dedisp.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include "data/search_mode_file.hpp"
#include "data/multi_timeseries.hpp"
#include "data/data_buffer.hpp"
#include <type_traits>
#include <memory>

typedef float DedispOutputType;

namespace OPS {


    class Dedisperser
    {
        dedisp_plan plan;
        unsigned int numGpus;

        std::shared_ptr<IO::SearchModeFile> searchModeFile;

        std::vector<float> dmList;
        float maxDelaySamples;

        std::vector<DEDISP_BOOL> killmask;
        std::unique_ptr<IO::MultiTimeSeries> multiTimeSeries;

        bool writeToFile;

        

    public:
    
        static void populateDMList(std::vector<float>& dmList, const std::string dmFile);
        static void populateDMList(std::vector<float>& dmList, float dmStart, float dmEnd, double width, double dmTol, 
                        double tSamp, double f0, double channelBW, int nChans);

    public:
        Dedisperser(std::shared_ptr<IO::SearchModeFile> searchModeFile, unsigned int numGpus, std::vector<float> dmList, 
                        std::string outputDir, std::string outputPrefix, std::string outputSuffix, 
                        std::string outputFormat);
        Dedisperser(std::shared_ptr<IO::SearchModeFile> searchModeFile, unsigned int numGpus, std::vector<float> dmList);

        virtual ~Dedisperser()
        {
            dedisp_destroy_plan(plan);
        }

        std::vector<float> getDMList()
        {
            return dmList;
        }
        std::size_t getMaxDelaySamples(){
            return maxDelaySamples;
        }   

        void setDMList(std::vector<float>& dmList);
        void setKillMask(std::vector<int> killmask_in);
        void setKillMask(std::string fileName);
        void writeData();
        // void dedisperse(IO::SearchModeFile *search_file, DEDISP_OUTPUT_TYPE *out_data);
        // void dedisperse(DEDISP_BYTE* input_data, DEDISP_OUTPUT_TYPE* out_data);

        void dedisperse(std::size_t startByte, std::size_t nBytesToRead){

            searchModeFile->readNBytes(startByte, nBytesToRead);

            std::size_t nSamplesIn = searchModeFile->bytesToSamples(nBytesToRead);
            std::size_t nSamplesOut = nSamplesIn - maxDelaySamples;

            std::shared_ptr<std::vector<DEDISP_OUTPUT_TYPE>> dedispersedData = multiTimeSeries->getCurrentDedispersedDataPtr();


            dedisp_error error;
            switch (searchModeFile->getValueForKey<int>(NBITS)) // check and shorten this to reinterpret_cast on getting buffer.. 
            {
                case 1:
                case 2:
                case 4:
                case 8:
                {
                    std::shared_ptr<std::vector<uint8_t>> inBuffer = searchModeFile->container->getBuffer<uint8_t>();
                    error = dedisp_execute(plan, nSamplesIn, inBuffer->data(), 8, reinterpret_cast<uint8_t*>(dedispersedData->data()),  32, (unsigned) 0);
                    break;
                }
                case 16:
                {
                    std::shared_ptr<std::vector<short>> inBuffer = searchModeFile->container->getBuffer<short>();
                    error = dedisp_execute(plan, nSamplesIn, reinterpret_cast<uint8_t*>(inBuffer->data()), 16, reinterpret_cast<uint8_t*>(dedispersedData->data()),  32, (unsigned) 0);
                    break;
                }
                case 32:
                {
                    std::shared_ptr<std::vector<int>> inBuffer = searchModeFile->container->getBuffer<int>();
                    error = dedisp_execute(plan, nSamplesIn, reinterpret_cast<uint8_t*>(inBuffer->data()), 32, reinterpret_cast<uint8_t*>(dedispersedData->data()),  32, (unsigned) 0);
                    break;
                }
                default:
                    break;
            }
            ErrorChecker::check_dedisp_error(error,"execute");
            multiTimeSeries->flush();

        }
        



    };
};