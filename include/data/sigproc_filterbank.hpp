#pragma once
#include "data/constants.hpp"
#include "data/search_mode_file.hpp"
#include "data/header_params.hpp"
#include "data/data_buffer.hpp"
#include <string>
#include <vector>
#include <memory>


namespace IO
{
    class SigprocFilterbank : public SearchModeFile
    {

    public:
        SigprocFilterbank(std::string file_name, std::string mode);

    public:
        bool verbose;
        std::size_t header_size;
        void readHeaderKeys();

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
            
            std::shared_ptr<std::vector<DTYPE>> buffer = this->container->getBuffer<DTYPE>();
            unsigned int nBits = this->nBits;
            this->goToByte(startByte);

            if (nBits >= 8) { // easy, just read the whole thing into buffer directly
                readFromFileAndVerify<DTYPE>(this->dataFile, nBytesOnDisk, buffer->data());
                return;
            }

            else {
                int bufferIdx = 0;
                /* In this case, we read nBytesOnDisk from disk using temporary buffer, and convert to nBytesOnRam */
                std::unique_ptr<std::vector<uint8_t>> tempBuffer = std::make_unique<std::vector<uint8_t>>(nBytesOnDisk);
                readFromFileAndVerify<uint8_t>(this->dataFile, nBytesOnDisk,tempBuffer->data());

                if (nBits < 8) { // eg: 2 bits
                    for (int byte = 0; byte < nBytesOnDisk; byte++) { // for each byte
                        for (int bitGroup = 0; bitGroup < BITS_PER_BYTE / nBits; bitGroup++) { 
                            buffer->at(bufferIdx) = extractBitsFromByte(tempBuffer->at(byte), static_cast<uint8_t>(bitGroup * nBits), static_cast<uint8_t>((bitGroup + 1) * nBits)); 
                            bufferIdx++;
                        }
                       
                    }
                }
                
            }

        }


        template<typename DTYPE, typename = std::enable_if_t<std::is_arithmetic<DTYPE>::value>>
        void writeNBytesOfType(){
            unsigned int nBits = this->nBits;
            this->nBytesOnDisk = this->nBytesOnDisk * BITS_PER_BYTE / nBits;
            this->nBytesOnRam = this->container->getNBytes();
            /* get the buffer that already has the data*/
            std::shared_ptr<std::vector<DTYPE>> inBuffer = this->container->getBuffer<DTYPE>();
            size_t inBufferSize = this->container->getNElements();
            /* At this point all necessary conversions have been done to data >=8 bits, directly write the results to a file*/
            if(nBits>=8){
                writeToFileAndVerify<DTYPE>(this->dataFile, inBufferSize, inBuffer.get());
                return;
            }

            /*compute minimum and maximum for current datatype and datatype to write to */
            int inTypeMin = std::numeric_limits<DTYPE>::min();  
            int inTypeMax = std::numeric_limits<DTYPE>::max();
            int inRange = inTypeMax - inTypeMin;
            int outTypeMin = 0;
            int outTypeMax = std::pow(2, nBits) - 1;
            int outRange = outTypeMax - outTypeMin;

            
            std::unique_ptr<std::vector<uint8_t>> outBuffer = std::make_unique<std::vector<uint8_t>>(this->nBytesOnDisk);
            std::size_t outIdx = 0;

            for(std::size_t inIdx = 0; inIdx < inBufferSize; ++inIdx) {
                outIdx = static_cast<int>(inIdx * nBits / BITS_PER_BYTE);
                DTYPE inBufferVal = inBuffer->at(inIdx);
              
                if (inBufferVal > outTypeMax) inBufferVal = inBufferVal > outTypeMax ? 
                                                                outTypeMax : 
                                                                outTypeMin + (inBufferVal - inTypeMin)/ inRange * outRange; 
                outBuffer->at(outIdx) |= inBufferVal << (inIdx * nBits % BITS_PER_BYTE);

            }
                
            
        }

        // template <typename ITYPE, typename OTYPE>
        // void copy_data(std::shared_ptr<DataBuffer<ITYPE>> inBuffer, std::shared_ptr<DataBuffer<OTYPE>> outBuffer){
        //     std::size_t inBufferSize = inBuffer->getNElements();
        //     std::size_t outBufferSize = outBuffer->getNElements();

        //     std::shared_ptr<ITYPE []> inBuffer = std::dynamic_pointer_cast<ITYPE[]>(inBuffer->getBuffer());
        //     std::shared_ptr<OTYPE []> outBuffer = std::dynamic_pointer_cast<OTYPE[]>(outBuffer->getBuffer());
            
        //     for(std::size_t i = 0; i < inBufferSize; ++i){
        //         outBuffer[i] = inBuffer[i];
        //     } 
            
            
            
        // }
        uint8_t extractBitsFromByte(uint8_t byte, uint8_t b1, uint8_t b2);

        void rewind_to_data_start();

        // void plot_data(std::size_t start_sample, std::size_t num_samples, bool fscrunch);
        int read_int();
        double read_double();
        std::string read_string();
        std::size_t read_num_bytes(std::size_t nBytes, char *bytes);
        std::size_t read_num_bytesToRead();
    };


    


    class SigprocTimeSeries : public IO::SigprocFilterbank
    {
    public:
        SigprocTimeSeries(std::string fileName, std::string mode) : SigprocFilterbank(fileName, mode)
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
