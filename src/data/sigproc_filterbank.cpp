#include "data/sigproc_filterbank.hpp"
#include "utils/sigproc_utils.hpp"
#include "data/search_mode_file.hpp"
#include "data/constants.hpp"
#include "utils/gen_utils.hpp"
#include <string>
#include <math.h>
#include <cstring>

/**
 * @brief Constructs a SigprocFilterbank object.
 *
 * @param file_name The name of the file.
 * @param mode The mode in which the file should be opened.
 */
IO::SigprocFilterbank::SigprocFilterbank(std::string file_name, std::string mode): SearchModeFile(file_name, mode)
{

    readHeaderKeys();
    this->headerFileOpenMode = mode;
    this->dataFileOpenMode = mode;    

    if (mode == READ)
    {
        readHeader();
        this->nBytesOnDisk = this->nSamps * this->nChans * this->nBits / BITS_PER_BYTE;
        this->nBytesOnRam = this->nBytesOnDisk * BITS_PER_BYTE / this->nBits;
    }
    else if (mode == WRITE)
    {
        std::cerr << "Nothing to initialise for " << mode << " mode for file: " << file_name << std::endl;
    }
}

/**
 * @brief Reads the header keys of a Sigproc file.
 *
 * This function is responsible for reading the header keys of a Sigproc file.
 *
 * @return int Returns 0 if the header keys are successfully read, otherwise returns an error code.
 */
void IO::SigprocFilterbank::readHeaderKeys()
{
    std::ifstream headerKeysFile(HEADER_KEYS_FILE);
    std::string line;

    while (std::getline(headerKeysFile, line))
    {
        std::istringstream lineStream(line);
        std::string key;
        std::string dtype;
        if (!(lineStream >> key >> dtype))
        {
            fprintf(stderr, "Error parsing header_keys file at this line: %s \n", line.c_str());
            break;
        }
        if (!dtype.compare(std::string(INT)))
        {
            headerParams.insert(std::make_pair(key, new HeaderParam<int>(key, dtype)));
        }
        else if (!dtype.compare(std::string(DOUBLE)))
        {
             headerParams.insert(std::make_pair(key, new HeaderParam<double>(key, dtype)));
        }
        else if (!dtype.compare(std::string(STRING)))
        {
            headerParams.insert(std::make_pair(key, new HeaderParam<char *>(key, dtype)));
        }
        else if (!dtype.compare(std::string(NULL_STR)))
        {
            headerParams.insert(std::make_pair(key, new HeaderParam<char *>(key, dtype)));
        }
    }

}





bool IO::SigprocFilterbank::isHeaderSeparate()
{
    return false;
}
/**
 * @brief Reads the header of a Sigproc file.
 *
 * This function reads the header of a Sigproc file and extracts relevant information such as the number of channels,
 * number of bits, number of samples, sampling time, and other header parameters. It also calculates the total observation time.
 *
 * @return The number of bytes in the header.
 */
void IO::SigprocFilterbank::readHeader()
{

    openHeaderFile();
    rewind(headerFile);

    int iter = 0;
    bool header_started = false;
    while (1)
    {
        iter++;
        int num_bytes = read_int();
        char header_key_bytes[num_bytes + 1];
        char dummy[] = "NULL";
        if (read_num_bytes(num_bytes, &header_key_bytes[0]))
        {

            if (!std::strcmp(header_key_bytes, HEADER_START.c_str()))
            {
                HeaderParamBase *header_param = getHeaderParam(header_key_bytes);
                static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                header_started = true;
                continue;
            }
            if (header_started)
            {
                if (!std::strcmp(header_key_bytes, HEADER_END.c_str()))
                {
                    HeaderParamBase *header_param = getHeaderParam(header_key_bytes);
                    static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                    header_started = false;
                    headerBytes = ftell(headerFile);
                    int inp_num;
                    inp_num = fileno(headerFile);
                    if ((fstat(inp_num, &headerFileStat)) < 0)
                    {
                        std::cerr << "ERROR:  could not fstat file: " << headerFile << std::endl;
                        break;
                    }
                    dataBytes = headerFileStat.st_size - headerBytes;
                    int nChans = getValueForKey<int>(NCHANS);
                    int nBits = getValueForKey<int>(NBITS);
                    int nifs = getValueForKey<int>(NIFS);
                    double tsamp = getValueForKey<double>(TSAMP);
                    int nbytes = nBits / 8;
                    long nsamples = dataBytes / (nChans * nbytes * nifs);
                    double tobs = nsamples * tsamp;
                    addToHeader<long>(NSAMPLES, LONG, nsamples);
                    addToHeader<double>(TOBS, DOUBLE, tobs);

                    this->nChans = nChans;
                    this->nBits = nBits;
                    this->nSamps = nsamples;
                    this->tsamp = tsamp;
                    this->fch1 = getValueForKey<double>(FCH1);
                    this->foff = getValueForKey<double>(FOFF);
                    break;
                }

                else
                {
                    std::string header_key(&header_key_bytes[0]);
                    HeaderParamBase *header_param = getHeaderParam(header_key_bytes);
                    std::string dtype = header_param->dtype;
                    header_param->inheader = true;
                    if (dtype == std::string(INT))
                    {
                        static_cast<HeaderParam<int> *>(header_param)->value = read_int();
                    }
                    else if (dtype == std::string(DOUBLE))
                    {
                        static_cast<HeaderParam<double> *>(header_param)->value = read_double();
                    }
                    else if (dtype == std::string(STRING))
                    {
                        num_bytes = read_int();
                        if (num_bytes == 0)
                        {
                            static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                            continue;
                        }
                        char *header_value_bytes = new char[num_bytes + 1];
                        if (read_num_bytes(num_bytes, header_value_bytes))
                        {
                            static_cast<HeaderParam<char *> *>(header_param)->value = header_value_bytes;
                        }
                    }
                    else if (dtype == std::string(NULL_STR))
                    {
                        num_bytes = read_int();
                        if (num_bytes == 0)
                        {
                            static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                            continue;
                        }
                    }
                }
            }
        }
    }

    this->headerBytes = headerBytes;
}

void IO::SigprocFilterbank::writeHeader()
{
    if (this->headerFileOpenMode != std::string((char *)WRITE) && this->headerFileOpenMode != std::string((char *)VIRTUALFIL))
    {
        std::cerr << " File not opened in write mode aborting now. " << std::endl;
    }

    int headerStartLength = HEADER_START.length();
    fwrite(&headerStartLength, sizeof(int), 1, headerFile);
    fwrite(HEADER_START.c_str(), sizeof(char), headerStartLength, headerFile);

    

    for (const auto& [key, base] : headerParams) {
        if(!key.compare(HEADER_START) || !key.compare(HEADER_END) || !key.compare(NSAMPLES) || !key.compare(TOBS) || !base->inheader) continue;
        int keyLength = key.length();
        fwrite(&keyLength, sizeof(int), 1, headerFile);
        fwrite(key.c_str(), sizeof(char), keyLength, headerFile);

        if (base->dtype == std::string(INT)) {
            int value = (static_cast<HeaderParam<int> *>(base))->value;
            fwrite(&value, sizeof(value), 1, headerFile);
        }
        else if (base->dtype == std::string(DOUBLE)) {
            double value = (static_cast<HeaderParam<double> *>(base))->value;
            fwrite(&value, sizeof(value), 1, headerFile);
        }
        else if (base->dtype == std::string(STRING)) {
            char *value = (static_cast<HeaderParam<char *> *>(base))->value;
            if (value != NULL) {
                int value_length = std::strlen(value);
                fwrite(&value_length, sizeof(int), 1, headerFile);
                fwrite(value, sizeof(*value), value_length, headerFile);
            }
            else {
                int value_length = 0;
                fwrite(&value_length, sizeof(value_length), 1, headerFile);
            }
        }
        else if (base->dtype == std::string(NULL_STR)){
            int null = 0;
            fwrite(&null, sizeof(null), 1, headerFile);
        }
    }

    int headerEndLength = HEADER_END.length();
    fwrite(&headerEndLength, sizeof(int), 1, headerFile);
    fwrite(HEADER_END.c_str(), sizeof(char), headerEndLength, headerFile);
}

void IO::SigprocFilterbank::readAllData()
{
    readNBytes(0, this->nBytesOnDisk);
}

void IO::SigprocFilterbank::readNBytes(std::size_t startByte, std::size_t nBytes) {
    this->nBytesOnDisk = nBytes;
    this->nBytesOnRam = this->nBytesOnDisk * BITS_PER_BYTE / this->nBits;
    switch (this->nBits)
    {
    case 1:
    case 2:
    case 4:
    case 8:
        this->readNBytesOfType<SIGPROC_FILTERBANK_8_BIT_TYPE>(startByte, nBytes);
        break;
    case 16:
        this->readNBytesOfType<SIGPROC_FILTERBANK_16_BIT_TYPE>(startByte, nBytes);
        break;
    case 32:
        this->readNBytesOfType<SIGPROC_FILTERBANK_32_BIT_TYPE>(startByte, nBytes);
        break;
    }

}

void IO::SigprocFilterbank::writeAllData() {}

void IO::SigprocFilterbank::rewind_to_data_start()
{
    fseek(this->dataFile, this->headerBytes, SEEK_SET);
}


int IO::SigprocFilterbank::read_int()
{
    int result;
    std::size_t count = fread(&result, sizeof(result), 1, dataFile);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}

double IO::SigprocFilterbank::read_double()
{
    double result;
    std::size_t count = fread(&result, sizeof(result), 1, dataFile);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}
