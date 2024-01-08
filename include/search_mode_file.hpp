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
#include "constants.hpp"
#include "header_params.hpp"

namespace FileTypes
{

    class SearchModeFile
    {

    public:

        /**
         * Variables related to the header
        */

        std::string header_file_name;
        bool header_file_open = false;
        FILE *header_file;
        struct stat header_file_stat;
        unsigned long header_bytes;
        std::vector<HeaderParamBase *> header_params;

        unsigned long nsamps;
        unsigned int nchans;
        unsigned char nbits;
        float fch1;
        float foff;
        float tsamp;      

        /**
         * Variables related to the data
        */

        std::string data_file_name;
        bool data_file_open = false;
        FILE *data_file;
        struct stat data_file_stat;        
        unsigned long data_bytes;
        unsigned char *data;  

        /**
         * Manipulations of the Header map
        */

        template <typename T>
        inline T get_value_for_key(const char *key)
        {
            HeaderParamBase *base = get_header_param(key);

            return static_cast<HeaderParam<T> *>(base)->value;
        }
        template <typename T>
        inline T get_value_or_default_for_key(const char *key, T default_value)
        {
            HeaderParamBase *base = get_header_param(key);
            if (base == NULL)
                return default_value;

            return static_cast<HeaderParam<T> *>(base)->value;
        }

        template <typename T>
        inline void set_value_for_key(const char *key, T value)
        {
            HeaderParamBase *base = get_header_param(key);
            if (base != NULL)
            {
                static_cast<HeaderParam<T> *>(base)->value = value;
            }
        }

        void print_header();
        void pretty_print_header();
        HeaderParamBase *get_header_param(std::string key);
        int remove_header_param(std::string key);

        /**
         * Virtual functions to be implemented by the child classes
        */

        virtual void open_header_file() = 0;
        virtual void close_header_file() = 0;

        virtual void open_data_file() = 0;
        virtual void close_data_file() = 0;

        virtual bool is_header_separate() = 0;

        virtual void read_header() = 0;
        virtual void write_header() = 0;

        virtual void read_all_data() =0;
        virtual void write_all_data()=0;

        virtual void read_nbytes(long start_byte = 0, long nbytes, unsigned char* data) = 0;
        virtual void write_nbytes(unsigned char* data) = 0;

        virtual void read_nsamps(long start_sample = 0, long nsamps, unsigned char* data) = 0;
        virtual void write_nsamps(unsigned char* data) = 0;

        virtual void read_nsecs(double start_sec = 0, double nsecs, unsigned char* data) = 0;
        virtual void write_nsecs(unsigned char* data) = 0;


        

        ~SearchModeFile()
        {

            for (std::vector<HeaderParamBase *>::iterator it = header_params.begin(); it != header_params.end(); ++it)
                delete *it;

            delete[] data;

            if (header_file)
                fclose(header_file);

            if (data_file)
                fclose(data_file);
        }
    };

    // void SearchModeFile::print_header() {
    //     for (const auto& header_param : header_params) {
    //         if (header_param != nullptr) {
    //             header_param->print();
    //         }
    //     }
    // }

};
