#include "data/sigproc_file.hpp"
#include "utils/sigproc_utils.hpp"
#include "data/search_mode_file.hpp"
#include "data/constants.hpp"
#include "utils/gen_utils.hpp"
#include <string>
#include <math.h>
#include <cstring>

/**
 * @brief Constructs a SigprocFile object.
 *
 * @param file_name The name of the file.
 * @param mode The mode in which the file should be opened.
 */
IO::SigprocFile::SigprocFile(std::string file_name, std::string mode)
{

    read_header_keys();

    if (mode == FILREAD)
    {
        read_header();
        this->nBytesFromDisk = this->nSamps * this->nChans * this->nBits / BITS_PER_BYTE;
        this->nBytesOnRam = this->nBytesFromDisk * BITS_PER_BYTE / this->nBits;
    }
    else if (mode == FILWRITE)
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
int IO::SigprocFile::read_header_keys()
{
    std::ifstream header_keys_file(HEADER_KEYS_FILE);
    std::string line;

    while (std::getline(header_keys_file, line))
    {
        std::istringstream line_Stream(line);
        std::string key;
        std::string dtype;
        if (!(line_Stream >> key >> dtype))
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

    return EXIT_SUCCESS;
}

/**
 * Opens the header file, if different from the data file.
 */
void IO::SigprocFile::open_headerFile()
{
    if (this->headerFileOpen || !this->is_header_separate)
        return;

    fileOpen(&headerFile, this->headerFileName, this->mode);
    this->headerFileOpen = true;
}

/**
 * Closes the header file associated with the SigprocFile object.
 */
void IO::SigprocFile::close_headerFile()
{
    fclose(headerFile);
    this->headerFileOpen = false;
    return EXIT_SUCCESS;
}

void IO::SigprocFile::open_dataFile()
{

    if (this->dataFileOpen)
        return return;

    fileOpen(&dataFile, this->dataFileName, this->mode);
    this->dataFileOpen = true;
}

void IO::SigprocFile::close_dataFile()
{
    fclose(dataFile);
    this->dataFileOpen = false;
    return EXIT_SUCCESS;
}

bool IO::SigprocFile::is_header_separate()
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
void IO::SigprocFile::read_header()
{

    if(! is_headerFileOpen){
        open_headerFile();
    }

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

            if (!std::strcmp(header_key_bytes, HEADER_START))
            {
                HeaderParamBase *header_param = getHeaderParam(header_key_bytes);
                static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                header_started = true;
                continue;
            }
            if (header_started)
            {
                if (!std::strcmp(header_key_bytes, HEADER_END))
                {
                    HeaderParamBase *header_param = getHeaderParam(header_key_bytes);
                    static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                    header_started = false;
                    headerBytes = ftell(headerFile);
                    int inp_num;
                    inp_num = fileno(headerFile);
                    if ((fstat(inp_num, &filestat)) < 0)
                    {
                        std::cerr << "ERROR:  could not fstat file: " << headerFile << std::endl;
                        break;
                    }
                    data_bytes = filestat.st_size - headerBytes;
                    int nChans = getValueForKey<int>(NCHANS);
                    int nBits = getValueForKey<int>(NBITS);
                    int nifs = getValueForKey<int>(NIFS);
                    double tsamp = getValueForKey<double>(TSAMP);
                    int nbytes = nBits / 8;
                    long nsamples = data_bytes / (nChans * nbytes * nifs);
                    double tobs = nsamples * tsamp;
                    add_to_header<long>(NSAMPLES, LONG, nsamples);
                    add_to_header<double>(TOBS, DOUBLE, tobs);

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

    this->header_size = headerBytes;
}

void IO::SigprocFile::write_header()
{
    if (mode != std::string((char *)FILWRITE) && mode != std::string((char *)VIRTUALFIL))
    {
        std::cerr << " File not opened in write mode aborting now. " << std::endl;
    }

    int header_start_length = std::strlen(HEADER_START);
    fwrite(&header_start_length, sizeof(int), 1, headerFile);
    fwrite(HEADER_START, sizeof(char), header_start_length, headerFile);
    for (std::vector<HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
    {
        HeaderParamBase *base = *it;

        const char *key = (*it)->key.c_str();
        if (!std::strcmp(key, HEADER_START) || !std::strcmp(key, HEADER_END) || !std::strcmp(key, NSAMPLES) || !std::strcmp(key, TOBS) || !base->inheader)
            continue;
        int key_length = std::strlen(key);
        fwrite(&key_length, sizeof(int), 1, headerFile);
        fwrite(key, sizeof(char), key_length, headerFile);

        if (base->dtype == std::string(INT))
        {
            int value = (static_cast<HeaderParam<int> *>(base))->value;
            fwrite(&value, sizeof(value), 1, headerFile);
        }
        else if (base->dtype == std::string(DOUBLE))
        {
            double value = (static_cast<HeaderParam<double> *>(base))->value;
            fwrite(&value, sizeof(value), 1, headerFile);
        }
        else if (base->dtype == std::string(STRING))
        {
            char *value = (static_cast<HeaderParam<char *> *>(base))->value;
            if (value != NULL)
            {
                int value_length = std::strlen(value);
                fwrite(&value_length, sizeof(int), 1, headerFile);
                fwrite(value, sizeof(*value), value_length, headerFile);
            }
            else
            {
                int value_length = 0;
                fwrite(&value_length, sizeof(value_length), 1, headerFile);
            }
        }
        else if (base->dtype == std::string(NULL_STR))
        {
            int null = 0;
            fwrite(&null, sizeof(null), 1, headerFile);
        }
    }

    int header_end_length = std::strlen(HEADER_END);
    fwrite(&header_end_length, sizeof(int), 1, headerFile);
    fwrite(HEADER_END, sizeof(char), header_end_length, headerFile);
}

void IO::SigprocFile::read_all_data()
{
    readNBytes(this->nBytesFromDisk);
}


unsigned char extract_bits_to_byte(unsigned char byte, unsigned char b1, unsigned char b2)
{
    // Create a mask with ones in the positions from b1 to b2
    unsigned char mask = ((1 << (b2 - b1 + 1)) - 1) << b1;

    // Apply the mask to the byte and shift the result to the rightmost position
    unsigned char result = (byte & mask) >> b1;

    return result;
}
void IO::SigprocFile::readNBytes(std::size_t startByte, std::size_t nBytes)
{
    if (this->data_buffer != NULL)
        throw InvalidBufferStateException("Buffer already allocated, cannot allocate again");

    this->nBytesFromDisk = nBytes;
    this->nBytesOnRam = nBytes * BITS_PER_BYTE / this->nBits;
    this->data_buffer = safeNew1D<DTYPE>(nBytesOnRam, __func__);

    double nBits = this->nBits;

    // go to start byte
    fseek(dataFile, startByte, SEEK_SET);

    if (nBits == 8)
    { // easy, just read the whole thing into buffer directly
        std::size_t count = fread(this->data_buffer->buffer, sizeof(DTYPE), nBytesFromDisk, this->dataFile);
        static_assert(count == nBytes, "Error in reading data, nbytes != count");
        return;
    }

    std::size_t ngulps = this->gulpSize > 0 ? nBytes / this->gulpSize : nBytes std::size_t total_bytes_read = 0;
    std::size_t global_idx = 0;

    while (total_bytes_read < nBytes)
    {

        std::size_t bytes_to_read = std::min(this->gulpSize, nBytes - total_bytes_read);

        if (bytes_to_read % nBits != 0)
        {
            bytes_to_read = (bytes_to_read / nBits - 1) * nBits; // to get edge cases and non multiples
        }

        DTYPE *temporary_buffer = safeNew1D<DTYPE>(bytes_to_read, __func__);
        std::size_t count = fread(temporary_buffer, sizeof(DTYPE), bytes_to_read, this->dataFile);
        static_assert(count == bytes_to_read, "Error in reading data, nbytes != bytes_to_read");

        if (nBits < 8)
        {
            for (int byte = 0; byte < bytes_to_read; byte++) // iterate over each byte
            {
                unsigned char byte_value = temporary_buffer[byte]; // get the byte value

                for (int bit_group = 0; bit_group < 8 / nBits; bit_group = bit_group++) // iterate over each bit group
                {

                    this->data_buffer->buffer[global_idx] = extract_bits_to_byte(byte_value, bit_group * nBits, (bit_group + 1) * nBits);
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



void IO::SigprocFile::write_all_data()
{

    fwrite(data, sizeof(unsigned char), data_bytes, dataFile);
}

void IO::SigprocFile::writeNBytes(std::size_t start_byte, std::size_t nbytes)
{
    if (this->data_buffer->buffer == NULL)
        throw InvalidBufferStateException("Buffer is not allocated, cannot write data");

    // go to start byte
    fseek(dataFile, start_byte, SEEK_SET);

    double nBits = this->nBits;

    if (nBits == 8)
    { // easy, just write the whole thing from buffer directly
        std::size_t count = fwrite(this->data_buffer->buffer, sizeof(DTYPE), nbytes, this->dataFile);
        static_assert(count == nbytes, "Error in writing data, nbytes != count");
        return;
    }

    std::size_t ngulps = this->gulpSize > 0 ? nbytes / this->gulpSize : nbytes;
    std::size_t total_bytes_written = 0;
    std::size_t global_idx = 0;

    while (total_bytes_written < nbytes)
    {
        std::size_t bytes_to_write = std::min(this->gulpSize, nbytes - total_bytes_written);

        if (bytes_to_write % nBits != 0)
        {
            bytes_to_write = (bytes_to_write / nBits - 1) * nBits; // to get around edge cases and non multiples, which should never occur
        }

        DTYPE *temporary_buffer = safeNew1D<DTYPE>(bytes_to_write, __func__);

        if (nBits < 8)
        {
            for (int byte = 0; byte < bytes_to_write; byte++) // iterate over each byte
            {
                unsigned char byte_value = 0;

                for (int bit_group = 0; bit_group < 8 / nBits; bit_group++) // iterate over each bit group
                {
                    byte_value |= (this->data_buffer->buffer[global_idx] << (bit_group * nBits));
                    global_idx++;
                }

                temporary_buffer[byte] = byte_value;
            }
        }
        else
        {
            int num_bytes_per_value = nBits / 8;

            while (global_idx < bytes_to_write)
            {
                for (int k = 0; k < num_bytes_per_value; k++)
                {
                    temporary_buffer[global_idx + k] = this->data_buffer->buffer[global_idx] / pow(2, 8 * (num_bytes_per_value - k));
                }
                global_idx++;
            }

            total_bytes_written += bytes_to_write;
        }

        std::size_t count = fwrite(temporary_buffer, sizeof(DTYPE), bytes_to_write, this->dataFile);
        static_assert(count == bytes_to_write, "Error in writing data, nbytes != bytes_to_write");

        delete[] temporary_buffer;
    }
}

void IO::SigprocFile::copy_data(std::size_t start_sample, std::size_t nsamples, DTYPE *data)
{

    int nChans = getValueForKey<int>(NCHANS);
    int nifs = getValueForKey<int>(NIFS);

    unsigned long bytes_to_copy = samplesToBytes(nsamples);
    unsigned long byte_to_start = samplesToBytes(start_sample) + this->headerBytes;

    std::memcpy(&this->data[data_bytes], &data[byte_to_start], sizeof(DTYPE) * bytes_to_copy);
    data_bytes += bytes_to_copy;
}

void IO::SigprocFile::write_all_data()
{
    writeNBytes(0, this->nBytesOnRam);
}

int IO::SigprocFile::rewind_to_data_start()
{
    fseek(this->dataFile, this->header_size, SEEK_SET);
}
void IO::SigprocFile::rewind_to_file_start()
{
    rewind(this->dataFile);
}

int IO::SigprocFile::read_int()
{
    int result;
    std::size_t count = fread(&result, sizeof(result), 1, dataFile);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}

double IO::SigprocFile::read_double()
{
    double result;
    std::size_t count = fread(&result, sizeof(result), 1, dataFile);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}
}
std::size_t IO::SigprocFile::read_num_bytes_to_read()
{
    int nbytes = 4;
    unsigned char bytes[nbytes];
    std::size_t count = fread(&bytes, nbytes, 1, dataFile);
    if (count)
        return (bytes[0] + bytes[1] * 256 + bytes[2] * 65536 + bytes[3] * 16777216);
    else
        return EXIT_FAILURE;
}
int IO::SigprocFile::read_num_bytes(int nbytes, char* bytes){
	size_t count = fread(bytes,nbytes,1,file);
	bytes[nbytes] = '\0';
	return count;

}