#pragma once
#include "data/constants.hpp"
#include "data/search_mode_file.hpp"
#include "data/header_params.hpp"
#include <string>
#include <vector>
#include <memory>


namespace IO
{
    class SigprocFile : public SearchModeFile
    {

    public:
        SigprocFile(std::string file_name, std::string mode);
        virtual ~SigprocFile() = 0;

    public:
        std::string mode;
        bool verbose;
        std::size_t header_size;
        void read_header_keys();

        void open_headerFile();
        void close_headerFile();

        void open_dataFile();
        void close_dataFile();

        bool is_header_separate();

        void read_header();
        void write_header();

        void read_all_data();
        void write_all_data();

        void readData(std::size_t start_byte, std::size_t nbytes) {
            this->nBytesFromDisk = nbytes;
            this->nBytesOnRam = this->nBytesFromDisk * BITS_PER_BYTE / this->nBits;
            this->container = std::make_unique<DataBufferBase>(start_byte, nBytesOnRam);

            switch (this->nBits)
            {
            case 1:
            case 2:
            case 4:
            case 8:
                this->readNBytes<SIGPROC_FILTERBANK_8_BIT_TYPE>(start_byte, nbytes);
                break;
            case 16:
                this->readNBytes<SIGPROC_FILTERBANK_16_BIT_TYPE>(start_byte, nbytes);
                break;
            case 32:
                this->readNBytes<SIGPROC_FILTERBANK_32_BIT_TYPE>(start_byte, nbytes);
                break;
            }
            

            
        }

        template <typename DTYPE>
        void readNBytes(std::size_t start_byte, std::size_t nbytes){

            this->nBytesFromDisk = nbytes;
            this->nBytesOnRam = this->nBytesFromDisk * BITS_PER_BYTE / this->nBits;
            this->container = std::make_unique<DTYPE>(start_byte, nBytesOnRam);
            
            std::shared_ptr<DTYPE> buffer = std::reinterpret_pointer_cast<DTYPE>(this->container->getBuffer());

            double nBits = this->nBits;

            // go to start byte
            fseek(dataFile, start_byte, SEEK_SET);

            if (nBits == 8)
            { // easy, just read the whole thing into buffer directly
                std::size_t count = fread(buffer, sizeof(DTYPE), nBytesFromDisk, this->dataFile);
                assert(count == nbytes, "Error in reading data, nbytes != count");
                return;
            }

            std::size_t ngulps = this->gulpSize > 0 ? nbytes / this->gulpSize : nbytes;
            std::size_t total_bytes_read = 0;
            std::size_t global_idx = 0;

            while (total_bytes_read < nbytes)
            {

                std::size_t bytes_to_read = std::min(this->gulpSize, nbytes - total_bytes_read);

                if (bytes_to_read % nBits != 0)
                {
                    bytes_to_read = (bytes_to_read / nBits - 1) * nBits; // to get edge cases and non multiples
                }

                std::unique_ptr<DTYPE> temporary_buffer = std::make_unique<DTYPE>(bytes_to_read);

                std::size_t count = fread(temporary_buffer, sizeof(DTYPE), bytes_to_read, this->dataFile);
                static_assert(count == bytes_to_read, "Error in reading data, nbytes != bytes_to_read");

                if (nBits < 8)
                {
                    for (int byte = 0; byte < bytes_to_read; byte++) // iterate over each byte
                    {
                        unsigned char byte_value = temporary_buffer[byte]; // get the byte value

                        for (int bit_group = 0; bit_group < 8 / nBits; bit_group = bit_group++) // iterate over each bit group
                        {

                            buffer[global_idx] = extract_bits_to_byte(byte_value, bit_group * nBits, (bit_group + 1) * nBits);
                            global_idx++;
                        }
                    }
                }
                else
                {
                    int num_bytes_per_value = nBits / 8;
                    // take nBits/8 bytes at a time and put them in the buffer
                    // for example, for 16 bits, convert to short and then convert to byte

                    while (ibyte < bytes_to_read)
                    {
                        std::size_t value = 0;
                        for (int k = 0; k < num_bytes_per_value; k++)
                        {
                            this->data_buffer->buffer[global_idx] += temporary_buffer[ibyte + k] * pow(2, 8 * (num_bytes_per_value - k));
                        }
                        global_idx++;
                    }
                }
                static_assert(global_idx == nBytesOnRam, "Error in reading data, nBytesOnRam != global_idx");
                delete[] temporary_buffer;
            }

        }
        template <typename DTYPE> 
        void writeNBytes(std::shared_ptr<DTYPE> ptr, std::size_t nBytes);

        template <typename DTYPE>
        void copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data);

        int rewind_to_data_start();
        int rewind_to_file_start();

        // void plot_data(std::size_t start_sample, std::size_t num_samples, bool fscrunch);
        int read_int();
        double read_double();
        std::string read_string();
        std::size_t read_num_bytes(std::size_t nbytes, char *bytes);
        std::size_t read_num_bytes_to_read();
    };


    class SigprocFilterbank : public IO::SigprocFile
    {
    
    };


    class SigprocTimeSeries : public IO::SigprocFile
    {
    public:
        SigprocTimeSeries(std::string file_name, std::string mode) : SigprocFile(file_name, mode)
        {
            // this->nChans = 1;
            // this->setValueForKey<int>(NCHANS, 1);
        }

    private:
        //  static int get_zero_dm_time_series(SigprocFilterbank *f, unsigned char *timeseries)
        // {

        //     for (int i = 0; i < f->nSamps; i++)
        //     {
        //         int add;
        //         for (int j = 0; j < f->nChans; j++)
        //         {
        //             add += f->data[i * f->nChans + j];
        //         }
        //         timeseries[i] = (unsigned char)(add);
        //     }

        //     return EXIT_SUCCESS;
        // }
    };
    class DedispersedSigProcTimeSeries : public IO::SigprocTimeSeries, public IO::DedispersedFile {};
}; // namespace IO
