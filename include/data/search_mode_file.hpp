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
         

        std::string dataFileName;
        bool dataFileOpen = false;
        FILE *dataFile;


        std::size_t nSamps;
        unsigned int nChans;
        unsigned int nBits;
        float fch1;
        float foff;
        float tsamp;

        std::size_t nBytesFromDisk;
        std::size_t nBytesOnRam;
        std::size_t gulpSize;
        std::unique_ptr<DataBufferBase> container;

        
        void print_header();
        void prettyPrint_header();
        HeaderParamBase *getHeaderParam(std::string key);
        int removeHeaderParam(std::string key);

        /**
         * Virtual functions to be implemented by the child classes
         */

        virtual void open_headerFile() = 0;
        virtual void close_headerFile() = 0;

        virtual void open_dataFile() = 0;
        virtual void close_dataFile() = 0;

        virtual bool is_header_separate() = 0;

        virtual void read_header() = 0;
        virtual void write_header() = 0;

        virtual void read_all_data() = 0;
        virtual void write_all_data() = 0;

        virtual void readNBytes(std::size_t start_byte = 0, std::size_t nbytes = 0) = 0;
        virtual void writeNBytes() = 0;

        void read_nSamps(std::size_t start_sample = 0, std::size_t nSamps = 0);
        void read_nsecs(double start_sec = 0, double nsecs = 0);

        void skip_samples(std::size_t num_skip_samples);
        void skip_bytes(std::size_t bytes);
        void skip_time(double time_sec);

        void goto_sample(int sample_number);
        void goto_timestamp(double timestamp_sec);
        void goto_byte(std::size_t byte);


        std::size_t samplesToBytes(std::size_t nsamples);
        std::size_t bytesToSamples(std::size_t nsamples);

        std::size_t timeToBytes(std::size_t nsecs);
        std::size_t getTotalDataSize();

        double get_mean();
        double get_rms();



        inline SearchModeFile(){

        }


        
        



        template <typename T>
        inline void add_to_header(const char *key, std::string dtype, T value)
        {
            HeaderParamBase *base = getHeaderParam(key);
            if (base != NULL)
            {
                static_cast<HeaderParam<T> *>(base)->value = value;
            }
            else
            {
                HeaderParam<T> *param = new HeaderParam<T>(key, dtype, value);
                headerParams.insert(std::pair<std::string, HeaderParamBase *>(key, param));
            }
        }
        template <typename T>
        inline T getValueForKey(const char *key)
        {
            HeaderParamBase *base = getHeaderParam(key);

            return static_cast<HeaderParam<T> *>(base)->value;
        }
        template <typename T>
        inline T getValueOrDefaultForKey(const char *key, T default_value)
        {
            HeaderParamBase *base = getHeaderParam(key);
            if (base == NULL)
                return default_value;

            return static_cast<HeaderParam<T> *>(base)->value;
        }

        template <typename T>
        inline void setValueForKey(const char *key, T value)
        {
            HeaderParamBase *base = getHeaderParam(key);
            if (base != NULL)
            {
                static_cast<HeaderParam<T> *>(base)->value = value;
            }
        }

        template <typename DTYPE>
        void copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data){
            int nChans = getValueForKey<int>(NCHANS);
            int nifs = getValueForKey<int>(NIFS);

            unsigned long bytes_to_copy = samplesToBytes(nsamples);
            unsigned long byte_to_start = samplesToBytes(start_sample) + this->headerBytes;

            std::memcpy(&this->container.getBuffer(), &data[byte_to_start], sizeof(DTYPE) * bytes_to_copy);
        }
        void nsamples_in_buffer();         


        std::size_t get_nSamps() const {
        return nSamps;
        }

        unsigned int getNchans() const {
            return nChans;
        }

        unsigned int get_nBits() const {
            return nBits;
        }

        float get_fch1() const {
            return fch1;
        }

        float get_foff() const {
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
