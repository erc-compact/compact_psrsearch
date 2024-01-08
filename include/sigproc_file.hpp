#pragma once
#include "constants.hpp"    
#include "search_mode_file.hpp"
#include "header_params.hpp"
#include <string>

namespace FileTypes    
{
    class SigprocFile : public FileTypes::SearchModeFile
    {

    public:
        SigprocFile(std::string file_name, std::string mode, bool verbose);
        

        template <typename T>
        inline void add_to_header(const char *key, std::string dtype, T value)
        {
            HeaderParamBase *base = get_header_param(key);
            if (base != NULL)
            {
                static_cast<HeaderParam<T> *>(base)->value = value;
            }
            else
            {
                HeaderParam<T> *param = new HeaderParam<T>(key, dtype, value);
                header_params.push_back(param);
            }
        }


       
    public:
        std::string mode;
        bool verbose;
        struct stat filestat;
        long header_size;

        int open_header_file();
        int open_data_file();
        int close_header_file();
        int close_data_file();
        bool header_in_file();
     
        int read_header_keys();
        void read_header();
        int read_all_data();
        int read_num_samples(int ngulp, unsigned char *data);
        int read_num_time(double duration, unsigned char *data);
        int read_nbytes(long nbytes, unsigned char *data);

        size_t samples_to_bytes(long nsamples);
        int skip_samples(long num_skip_samples);
        int skip_bytes(long bytes);
        int skip_time(double time_sec);

        int goto_sample(int sample_number);
        int goto_timestamp(double timestamp_sec);
        int goto_byte(long byte);

        void write_header();
        int store_data(unsigned char *data, unsigned long start_sample, unsigned long num_samples);

        int alloc_data();
        int rewind_to_data_start();
        int rewind_to_file_start();
        int unload_all_data_to_file();

        int load_all_data();
        double get_mean();
        double get_rms();
        void plot_data(long start_sample, long num_samples, bool fscrunch);
        int read_int();
        double read_double();
        std::string read_string();
        size_t read_num_bytes(size_t nbytes, char *bytes);
        size_t read_num_bytes_to_read();

    };
    

    class SigprocFilterbank : public FileTypes::SigprocFile {};
    class SigprocTimeSeries : public FileTypes::SigprocFile {
    public:
        SigprocTimeSeries(std::string file_name, std::string mode, bool verbose) : SigprocFile(file_name, mode, verbose)
        {
            nchans = 1;
            set_value_for_key<int>(NCHANS, 1);
            
        }
    private:
        //  static int get_zero_dm_time_series(SigprocFilterbank *f, unsigned char *timeseries)
        // {

        //     for (int i = 0; i < f->nsamps; i++)
        //     {
        //         int add;
        //         for (int j = 0; j < f->nchans; j++)
        //         {
        //             add += f->data[i * f->nchans + j];
        //         }
        //         timeseries[i] = (unsigned char)(add);
        //     }

        //     return EXIT_SUCCESS;
        // }
    }; 
}; // namespace FileTypes

