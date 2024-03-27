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
            static std::shared_ptr<SearchModeFile> createInstance(std::string fileName, std::string mode, std::string fileType);
            static inline std::string guessFileType(std::string file_name); 


        public:

            SearchModeFile(std::string fileName, std::string mode);

            std::string headerFileName;
            bool headerFileOpen = false;
            FILE *headerFile;
            std::size_t headerBytes;
            std::map<std::string, HeaderParamBase *> headerParams;         //map of header params
            std::string headerFileOpenMode;

            struct stat headerFileStat;
            struct stat dataFileStat;

            

            std::string dataFileName;
            bool dataFileOpen = false;
            FILE *dataFile;
            std::size_t dataBytes;
            std::string dataFileOpenMode;


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
            HeaderParamBase *getHeaderParam(const std::string key);
            void removeHeaderParam(const std::string key);

            /**
             * Virtual functions to be implemented by the child classes
             */

            void openHeaderFile();
            void closeHeaderFile();

            void openDataFile();
            void closeDataFile();

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

            void clearBuffer();


            std::size_t samplesToBytes(std::size_t nSamples);
            std::size_t bytesToSamples(std::size_t nSamples);

            std::size_t timeToBytes(std::size_t nSecs);
            std::size_t getTotalDataSize();

            double getMean();
            double getRMS();




            template <typename T>
            void addToHeader(const std::string key, const std::string dtype, T value) {
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
            T getValueForKey(const std::string key) {
                HeaderParamBase *base = getHeaderParam(key);
                return static_cast<HeaderParam<T> *>(base)->value;
            }
            template <typename T>
            T getValueOrDefaultForKey(const std::string key, T default_value) {
                HeaderParamBase *base = getHeaderParam(key);
                if (base == NULL) return default_value;
                return static_cast<HeaderParam<T> *>(base)->value;
            }

            template <typename T>
            inline void setValueForKey(const std::string key, T value){
                HeaderParamBase *base = getHeaderParam(key);
                if (base != NULL) static_cast<HeaderParam<T> *>(base)->value = value;
            }

            // template <typename DTYPE>
            // void copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data){
            //     int nChans = getValueForKey<int>(NCHANS);
            //     int nifs = getValueForKey<int>(NIFS);

            //     unsigned long bytes_to_copy = samplesToBytes(nsamples);
            //     unsigned long byte_to_start = samplesToBytes(start_sample) + this->headerBytes;

            //     std::memcpy(std::dynamic_pointer_cast<std::shared_ptr<DataBuffer>>(data)->getBuffer(), &data[byte_to_start], sizeof(DTYPE) * bytes_to_copy);
            // }
            std::size_t nSamplesInBuffer();         


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
