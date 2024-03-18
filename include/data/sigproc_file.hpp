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

    public:
        std::string mode;
        bool verbose;
        std::size_t header_size;
        void readHeaderKeys();

        void openHeaderFile();
        void closeHeaderFile();

        void openDataFile();
        void closeDataFile();

        bool isHeaderSeparate();

        void readHeader();
        void writeHeader();

        void readAllData();
        void writeAllData();

        void readNBytes(std::size_t startByte, std::size_t nBytes); 
        void writeNBytes(std::size_t startByte, std::size_t nBytes);

        template<typename DTYPE, typename = std::enable_if_t<std::is_arithmetic<DTYPE>::value>>
        void readNBytesOfType(std::size_t startByte, std::size_t nBytes){

            this->nBytesOnDisk = nBytes;
            this->nBytesOnRam = this->nBytesOnDisk * BITS_PER_BYTE / this->nBits;
            this->container = std::make_unique<DataBuffer<DTYPE>>(startByte, nBytesOnRam);
            
            std::shared_ptr<DTYPE []> buffer = std::dynamic_pointer_cast<DTYPE[]>(this->container->getBuffer());
            unsigned int nBits = this->nBits;
            this->goToByte(startByte);

            if (nBits == 8) { // easy, just read the whole thing into buffer directly
                readFromFileAndVerify<DTYPE>(this->dataFile, nBytesOnDisk, buffer.get());
                return;
            }

            else {
                int bufferIdx = 0;
                /* In this case, we read nBytesOnDisk from disk using temporary buffer, and convert to nBytesOnRam */
                std::unique_ptr<std::byte> tempBuffer = std::make_unique<std::byte>(nBytesOnDisk);
                readFromFileAndVerify<std::byte>(this->dataFile, nBytesOnDisk, tempBuffer.get());

                if (nBits < 8) { // eg: 2 bits
                    for (int byte = 0; byte < nBytesOnDisk; byte++) { // for each byte
                        for (int bitGroup = 0; bitGroup < BITS_PER_BYTE / nBits; bitGroup++) { 
                            buffer[bufferIdx] = extractBitsFromByte(tempBuffer[byte], bitGroup * nBits, (bitGroup + 1) * nBits); 
                        }
                    }
                }
                else
                {
                    unsigned char nBytesPerValue = nBits / BITS_PER_BYTE;   
                    unsigned char ibyte=0;
                    while (ibyte < nBytesOnDisk)
                    {
                        this->container->buffer[bufferIdx] = 0; // reset just in case it has some arbitrary undeleted previous value
                        for (int k = nBytesPerValue -1; k >= 0; k--) 
                                this->container->buffer[bufferIdx] |= static_cast<std::size_t>(tempBuffer[ibyte + k]) << (BITS_PER_BYTE * k);                        
                        ibyte+=nBytesPerValue;
                    }
                }
                bufferIdx++;
            }

        }

        template<typename DTYPE, typename = std::enable_if_t<std::is_arithmetic<DTYPE>::value>>
        void writeNBytesOfType(std::size_t startByte, std::size_t nBytes){

            this->nBytesOnDisk = nBytes;
            this->nBytesOnRam = this->nBytesOnDisk * BITS_PER_BYTE / this->nBits;
            std::shared_ptr<DTYPE []> buffer = std::dynamic_pointer_cast<DTYPE[]>(this->container->getBuffer());
            unsigned int nBits = this->nBits;
            this->goToByte(startByte);
            switch (this->nBits)
            {
            case 1:
            case 2:
            case 4:
            case 8:
                this->writeNBytesOfType<SIGPROC_FILTERBANK_8_BIT_TYPE>(startByte, nBytes);
                break;
            case 16:
                this->writeNBytesOfType<SIGPROC_FILTERBANK_16_BIT_TYPE>(startByte, nBytes);
                break;
            case 32:
                this->writeNBytesOfType<SIGPROC_FILTERBANK_32_BIT_TYPE>(startByte, nBytes);
                break;
            }
    
        }

            


        template <typename DTYPE>
        void copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data);

        int rewind_to_data_start();
        int rewind_to_file_start();

        // void plot_data(std::size_t start_sample, std::size_t num_samples, bool fscrunch);
        int read_int();
        double read_double();
        std::string read_string();
        std::size_t read_num_bytes(std::size_t nBytes, char *bytes);
        std::size_t read_num_bytesToRead();
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
