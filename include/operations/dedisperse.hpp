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
#include <type_traits>

typedef float DedispOutputType;

namespace OPS {

    class Dedisperser
    {
        dedisp_plan plan;
        IO::SearchModeFile *searchModeFile;
        unsigned int numGpus;
        std::vector<float> dmList;
        float maxDelaySamples;
        std::vector<DEDISP_BOOL> killmask;
        IO::MultiTimeSeries multiTimeSeries;
        bool writeToFile;

    public:
    
        static void generateDmList(std::string dm_file);
        static void generateDmList(float dm_start, float dm_end, float width, float tolerance);

    public:
            inline Dedisperser( IO::SearchModeFile *searchModeFile, unsigned int numGpus, std::vector<float> dmList, 
                            std::string outputDir, std::string outputPrefix, std::string outputSuffix, 
                            std::string outputFormat):
                Dedisperser(searchModeFile, numGpus, dmList) {


            dedisp_error error = dedisp_create_plan_multi(&plan,
                            searchModeFile->getValueForKey<std::size_t>(NSAMPS),
                            searchModeFile->getValueForKey<float>(TSAMP),
                            searchModeFile->getValueForKey<float>(FCH1),
                            searchModeFile->getValueForKey<float>(FOFF),
                            numGpus);

            ErrorChecker::check_dedisp_error(error,"create_plan_multi");

            this->maxDelaySamples = dedisp_get_max_delay(plan);

            std::size_t nSamplesOut = searchModeFile->bytesToSamples(nBytesToRead)  - maxDelaySamples;
            this->multiTimeSeries = IO::MultiTimeSeries(dmList, nSamplesOut, outputDir, outputPrefix, outputSuffix, outputFormat);
            this->writeToFile = true;
        }

        inline Dedisperser(IO::SearchModeFile *searchModeFile, unsigned int numGpus, std::vector<float> dmList){
            this->searchModeFile = searchModeFile;
            this->numGpus = numGpus;
            this->dmList = dmList;

            this->writeToFile = false;

            killmask.resize(searchModeFile->getNchans(),1);   

        }


        virtual ~Dedisperser()
        {
            dedisp_destroy_plan(plan);
        }

        std::vector<float> getDmList(void)
        {
            return dmList;
        }
        std::size_t getMaxDelaySamples(){
            return maxDelaySamples;
        }   

        void setDmList(float *dmList, unsigned int size);
        void setKillMask(std::vector<int> killmask_in);
        void setKillMask(std::string filename);
        void dedisperse(IO::SearchModeFile *search_file, DEDISP_OUTPUT_TYPE *out_data);
        void dedisperse(DEDISP_BYTE* input_data, DEDISP_OUTPUT_TYPE* out_data);

        template <typename DTYPE, typename std::enable_if<std::is_arithmetic<DTYPE>::value>::type>
        void dedisperse(std::size_t startByte, std::size_t nBytesToRead,  bool cast = true){

            searchModeFile->readNBytes(startByte, nBytesToRead);
            
            if(cast && !std::is_same<DTYPE, DEDISP_BYTE>::value) {
                std::shared_ptr<DTYPE[]> tempBuffer = std::reinterpret_pointer_cast<DTYPE[]>(searchModeFile->container->getBuffer());
                std::size_t nSamplesIn = searchModeFile->bytesToSamples(nBytesToRead);
                std::size_t nSamplesOut = nSamplesIn - maxDelaySamples;

                dedisp_error error = dedisp_execute(plan, nSamplesIn, tempBuffer.get(), 8, outData,  32, (unsigned) 0);
                ErrorChecker::check_dedisp_error(error,"execute");
                
            }
            else{
                dedisp_error error = dedisp_execute(plan, nSamplesIn, searchModeFile->container->getBuffer(), searchModeFile->getValueForKey("NBITS"), outData,  32, (unsigned) 0);
                ErrorChecker::check_dedisp_error(error,"execute");
            }
        }
        



    };
};