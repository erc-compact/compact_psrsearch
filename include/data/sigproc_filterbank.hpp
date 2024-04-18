#pragma once
#include "data/constants.hpp"
#include "data/search_mode_file.hpp"
#include "data/header_params.hpp"
#include "data/data_buffer.hpp"
#include "exceptions.hpp"
#include "utils/sigproc_utils.hpp"
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
        bool isHeaderSeparate();

        void readHeader();
        void writeHeader();

        void readAllData();
        void writeAllData();

        void readNBytes(std::size_t startByte, std::size_t nBytes); 
        void writeNBytes();

        template<typename DTYPE, typename = std::enable_if_t<std::is_arithmetic<DTYPE>::value>>
        void readNBytesOfType(std::size_t startByte, std::size_t nBytes){
            startByte += this->headerBytes;

            this->nBytesOnDisk = nBytes;
            this->nBytesOnRam = this->nBytesOnDisk * BITS_PER_BYTE / this->nBits;
            this->container = std::make_shared<DataBuffer<DTYPE>>(startByte, nBytesOnRam);
          
            
            std::shared_ptr<std::vector<DTYPE>> buffer = this->container->getBuffer<DTYPE>();

            unsigned int nBits = this->nBits;
            this->openDataFile();
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
                writeToFileAndVerify<DTYPE>(this->dataFile, inBufferSize, inBuffer->data());
                return;
            }
            else{
                //throw not implemented error
                throw FunctionalityNotImplemented("Write < 8 bit data");
            }

            
        }


        void rewindToDataStart();

        // void plot_data(std::size_t start_sample, std::size_t num_samples, bool fscrunch);
        int readInt();
        double readDouble();
        std::size_t readNumBytesFromHeader(int nbytes, char* bytes);
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
