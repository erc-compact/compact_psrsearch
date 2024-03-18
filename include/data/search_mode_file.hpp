#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "data/constants.hpp"
#include "data/header_params.hpp"
#include "data/data_buffer.hpp"
#include "exceptions.hpp"
#include <variant>
#include <memory>
#include <map>




namespace IO
{      
    class DedispersedFile{ 
        public: 
            float dm;
            DedispersedFile(float dm) : dm(dm) {}    
     };


    class SearchModeFile 
    {

        public:
            static SearchModeFile* createInstance(std::string file_name, std::string mode, std::string file_type);
            static inline std::string guessFileType(std::string file_name); 


        public:

            std::string headerFileName;
            bool headerFileOpen = false;
            FILE *headerFile;
            std::size_t headerBytes;
            std::map<std::string, HeaderParamBase *> headerParams;         //map of header params

            struct stat headerFileStat;
            struct stat dataFileStat;

            

            std::string dataFileName;
            bool dataFileOpen = false;
            FILE *dataFile;
            std::size_t dataBytes;


            std::size_t nSamps;
            unsigned int nChans;
            unsigned int nBits;
            float fch1;
            float foff;
            float tsamp;

            std::size_t nBytesOnDisk;
            std::size_t nBytesOnRam;
            std::size_t gulpSize;
            std::unique_ptr<DataBufferBase> container;
        

            
            void printHeader();
            void prettyPrintHeader();
            HeaderParamBase *getHeaderParam(std::string key);
            void removeHeaderParam(std::string key);

            /**
             * Virtual functions to be implemented by the child classes
             */

            virtual void openHeaderFile() = 0;
            virtual void closeHeaderFile() = 0;

            virtual void openDataFile() = 0;
            virtual void closeDataFile() = 0;

            virtual bool isHeaderSeparate() = 0;

            virtual void readHeader() = 0;
            virtual void writeHeader() = 0;

            virtual void readAllData() = 0;
            virtual void writeAllData() = 0;

            virtual void readNBytes(std::size_t start_byte = 0, std::size_t nbytes = 0) = 0;
            // virtual void writeNBytes() = 0;

            void readNSamps(std::size_t startSample = 0, std::size_t nSamps = 0);
            void readNSecs(double startSecs = 0, double nsecs = 0);

            void skipSamples(std::size_t numSkipSamples);
            void skipBytes(std::size_t bytes);
            void skipTime(double timeSecs);

            void gotoSample(std::size_t sampleNumber);
            void gotoTimestamp(double timeStampSecs);
            void goToByte(std::size_t byte);


            std::size_t samplesToBytes(std::size_t nSamples);
            std::size_t bytesToSamples(std::size_t nSamples);

            std::size_t timeToBytes(std::size_t nSecs);
            std::size_t getTotalDataSize();

            double getMean();
            double getRMS();



            inline SearchModeFile(){}

            template <typename T>
            void addToHeader(const char *key, std::string dtype, T value) {
                HeaderParamBase *base = getHeaderParam(key);
                if (base != NULL) {
                    static_cast<HeaderParam<T> *>(base)->value = value;
                }
                else {
                    HeaderParam<T> *param = new HeaderParam<T>(key, dtype, value);
                    headerParams.insert(std::pair<std::string, HeaderParamBase *>(key, param));
                }
            }
            
            template <typename T>
            T getValueForKey(const char *key) {
                HeaderParamBase *base = getHeaderParam(key);
                return static_cast<HeaderParam<T> *>(base)->value;
            }
            template <typename T>
            T getValueOrDefaultForKey(const char *key, T default_value) {
                HeaderParamBase *base = getHeaderParam(key);
                if (base == NULL) return default_value;
                return static_cast<HeaderParam<T> *>(base)->value;
            }

            template <typename T>
            inline void setValueForKey(const char *key, T value){
                HeaderParamBase *base = getHeaderParam(key);
                if (base != NULL) static_cast<HeaderParam<T> *>(base)->value = value;
            }

            template <typename DTYPE>
            void copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data){
                int nChans = getValueForKey<int>(NCHANS);
                int nifs = getValueForKey<int>(NIFS);

                unsigned long bytes_to_copy = samplesToBytes(nsamples);
                unsigned long byte_to_start = samplesToBytes(start_sample) + this->headerBytes;

                std::memcpy(this->container->getBuffer(), &data[byte_to_start], sizeof(DTYPE) * bytes_to_copy);
            }
            void nSamplesInBuffer();         


            std::size_t getNSamps() const {
            return nSamps;
            }

            unsigned int getNChans() const {
                return nChans;
            }

            unsigned int getNBits() const {
                return nBits;
            }

            float getFch1() const {
                return fch1;
            }

            float getFoff() const {
                return foff;
            }

            float getTsamp() const {
                return tsamp;
            }

            virtual ~SearchModeFile()
            {
            // for (std::vector<HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
            //     delete *it;
            
            for (std::map<std::string, HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
                delete it->second;
            
            if (headerFile)
                fclose(headerFile);

            if (dataFile)
                fclose(dataFile);


            }
            


    };

    
};
