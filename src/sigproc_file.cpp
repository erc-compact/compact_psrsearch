#include "sigproc_file.hpp"
#include "sigproc_utils.hpp"
#include "search_mode_file.hpp"
#include "gen_utils.hpp"
#include <string>
#include <math.h>
#include <cstring>

/**
 * @brief Constructs a SigprocFile object.
 * 
 * @param file_name The name of the file.
 * @param mode The mode in which the file should be opened.
 * @param verbose Whether to enable verbose output.
 */
FileTypes::SigprocFile::SigprocFile(std::string file_name, std::string mode, bool verbose)
{

    read_header_keys();

    if (mode == FILREAD)
    {
        read_header();
    }
    else if (mode == FILWRITE)
    {
        std::cerr << "Nothing to initialise for " << mode << " mode for file: " << file_name << std::endl;
    }

}

int FileTypes::SigprocFile::read_header_keys()
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
            header_params.push_back(new HeaderParam<int>(key, dtype));
        }
        else if (!dtype.compare(std::string(DOUBLE)))
        {
            header_params.push_back(new HeaderParam<double>(key, dtype));
        }
        else if (!dtype.compare(std::string(STRING)))
        {
            header_params.push_back(new HeaderParam<char *>(key, dtype));
        }
        else if (!dtype.compare(std::string(NULL_STR)))
        {
            header_params.push_back(new HeaderParam<char *>(key, dtype));
        }
    }

    return EXIT_SUCCESS;
}

void FileTypes::SigprocFile::open_header_file()
{
    
    file_open(&header_file, this->header_file_name, this->mode);
    this->is_header_file_open = true;
}



int FileTypes::SigprocFile::open_data_file()
{
    file_open(&data_file, this->data_file_name, this->mode);
    this->is_data_file_open = true;
}

int FileTypes::SigprocFile::close_header_file()
{
    fclose(header_file);
    return EXIT_SUCCESS;
}

int FileTypes::SigprocFile::close_data_file()
{
    fclose(data_file);
    return EXIT_SUCCESS;
}

bool FileTypes::SigprocFile::header_in_file()
{
    return true;
}




void FileTypes::SigprocFile::read_all_data()
{
    this->alloc_data();

    if (this->verbose)
        fprintf(stderr, " reading %lu bytes \n", nbytes);

    return read_nbytes(nbytes, data);
}

int FileTypes::SigprocFile::read_num_samples(int ngulp, unsigned char *data)
{
    int nchans = get_value_for_key<int>(NCHANS);
    int nifs = get_value_for_key<int>(NIFS);
    size_t count = fread(data, sizeof(unsigned char), nchans * nifs * ngulp, data_file);

    if (this->verbose)
        std::cerr << "bytes read:" << count << " req:" << nchans << "*" << nifs << "*" << ngulp << std::endl;
 
    return check_size(nchans * nifs * ngulp, count);
}

int FileTypes::SigprocFile::read_nbytes(long nbytes, unsigned char *data)
{
    size_t count = fread(data, sizeof(unsigned char), nbytes, data_file);
    return check_size(nbytes, count);
}

int FileTypes::SigprocFile::read_num_time(double ntime, unsigned char *data)
{
    int ngulp = ntime / get_value_for_key<double>(TSAMP);
    return read_num_samples(ngulp, data);
}
size_t FileTypes::SigprocFile::samples_to_bytes(long nsamples){
    int nchans = get_value_for_key<int>(NCHANS);
    int nifs = get_value_for_key<int>(NIFS);
    int nbits = get_value_for_key<int>(NBITS);
    int nbytes = nbits / 8;
    return nsamples * nchans * nifs * nbytes;
}

int FileTypes::SigprocFile::skip_samples(long num_skip_samples)
{
    long bytes_to_skip = samples_to_bytes(num_skip_samples); 
    return !fseek(data_file, bytes_to_skip, SEEK_CUR);
}

int FileTypes::SigprocFile::skip_bytes(long num_bytes)
{
    return !fseek(data_file, num_bytes, SEEK_CUR);
}

int FileTypes::SigprocFile::skip_time(double time_sec)
{
    int num_samples = time_sec / get_value_for_key<double>(TSAMP);
    return skip_samples(num_samples);
}

int FileTypes::SigprocFile::goto_sample(int sample_number)
{
    long bytes_now = ftell(data_file);
    long bytes_at_sample = samples_to_bytes(sample_number)+ this->header_bytes; 

    int diff = bytes_now - bytes_at_sample;
    return !fseek(data_file, diff, SEEK_CUR);
}
int FileTypes::SigprocFile::goto_timestamp(double timestamp_sec)
{
    int sample_number = timestamp_sec / get_value_for_key<double>(TSAMP);
    return goto_sample(sample_number);
}

int FileTypes::SigprocFile::goto_byte(long byte)
{
    return !fseek(data_file, byte, SEEK_SET);
}



int FileTypes::SigprocFile::store_data(unsigned char *data, unsigned long start_sample, unsigned long num_samples)
{

    int nchans = get_value_for_key<int>(NCHANS);
    int nifs = get_value_for_key<int>(NIFS);

    unsigned long bytes_to_copy = samples_to_bytes(num_samples);
    unsigned long byte_to_start = samples_to_bytes(start_sample) + this->header_bytes;

    if (this->verbose)
        fprintf(stderr, "have = %ld copying = %ld  from byte = %ld \n", this->data_bytes, bytes_to_copy, byte_to_start);

    std::memcpy(&this->data[data_bytes], &data[byte_to_start], sizeof(unsigned char) * bytes_to_copy);
    data_bytes += bytes_to_copy;

    if (this->verbose)
        fprintf(stderr, "new data_bytes: %ld \n", data_bytes);
    return EXIT_SUCCESS;
}

int FileTypes::SigprocFile::alloc_data()
{
    unsigned long size = this->nsamps * this->nchans * this->nbits / BITS_PER_BYTE;
    this->data = safe_new_1D<unsigned char>(size, " Loading all data");
    return EXIT_SUCCESS;
}

void FileTypes::SigprocFile::write_all_data() {

    if (this->verbose)
        fprintf(stderr, "writing %ld bytes \n", data_bytes);
    
    fwrite(data, sizeof(unsigned char), data_bytes, data_file);

}

/**
 * @brief Reads the header of a Sigproc file.
 * 
 * This function reads the header of a Sigproc file and extracts relevant information such as the number of channels,
 * number of bits, number of samples, sampling time, and other header parameters. It also calculates the total observation time.
 * 
 * @return The number of bytes in the header.
 */
void FileTypes::SigprocFile::read_header()
{

    rewind(data_file);

    int iter = 0;
    bool header_started = false;
    while (1)
    {
        iter++;
        int num_bytes = read_int();
        char header_key_bytes[num_bytes + 1];
        if (read_num_bytes(num_bytes, &header_key_bytes[0]))
        {

            if (!std::strcmp(header_key_bytes, HEADER_START))
            {
                HeaderParamBase *header_param = get_header_param(header_key_bytes);
                char dummy[] = "NULL";
                static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                header_started = true;
                continue;
            }
            if (header_started)
            {
                if (!std::strcmp(header_key_bytes, HEADER_END))
                {
                    HeaderParamBase *header_param = get_header_param(header_key_bytes);
                    char dummy[] = "NULL";
                    static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                    header_started = false;
                    header_bytes = ftell(header_file);
                    int inp_num;
                    inp_num = fileno(header_file);
                    if ((fstat(inp_num, &filestat)) < 0)
                    {
                        std::cerr << "ERROR:  could not fstat file: " << header_file << std::endl;
                        break;
                    }
                    data_bytes = filestat.st_size - header_bytes;
                    int nchans = get_value_for_key<int>(NCHANS);
                    int nbits = get_value_for_key<int>(NBITS);
                    int nifs = get_value_for_key<int>(NIFS);
                    double tsamp = get_value_for_key<double>(TSAMP);
                    int nbytes = nbits / 8;
                    long nsamples = data_bytes / (nchans * nbytes * nifs);
                    double tobs = nsamples * tsamp;
                    add_to_header<long>(NSAMPLES, LONG, nsamples);
                    add_to_header<double>(TOBS, DOUBLE, tobs);

                    this->nchans = nchans;
                    this->nbits = nbits;
                    this->nsamps = nsamples;
                    this->tsamp = tsamp;
                    this->fch1 = get_value_for_key<double>(FCH1);
                    this->foff = get_value_for_key<double>(FOFF);
                    break;
                }

                else
                {
                    std::string header_key(&header_key_bytes[0]);
                    HeaderParamBase *header_param = get_header_param(header_key_bytes);
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
                            char dummy[] = "NULL";
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
                            char dummy[] = "NULL";
                            static_cast<HeaderParam<char *> *>(header_param)->value = dummy;
                            continue;
                        }
                    }
                }
            }
        }
    }

    this->header_size = header_bytes;
}

void FileTypes::SigprocFile::write_header()
{
    if (mode != std::string((char *)FILWRITE) && mode != std::string((char *)VIRTUALFIL))
    {
        std::cerr << " File not opened in write mode aborting now. " << std::endl;
    }

    int header_start_length = std::strlen(HEADER_START);
    fwrite(&header_start_length, sizeof(int), 1, header_file);
    fwrite(HEADER_START, sizeof(char), header_start_length, header_file);
    for (std::vector<HeaderParamBase *>::iterator it = header_params.begin(); it != header_params.end(); ++it)
    {
        HeaderParamBase *base = *it;

        const char *key = (*it)->key.c_str();
        if (!std::strcmp(key, HEADER_START) || !std::strcmp(key, HEADER_END) || !std::strcmp(key, NSAMPLES) || !std::strcmp(key, TOBS) || !base->inheader)
            continue;
        int key_length = std::strlen(key);
        fwrite(&key_length, sizeof(int), 1, header_file);
        fwrite(key, sizeof(char), key_length, header_file);

        if (base->dtype == std::string(INT))
        {
            int value = (static_cast<HeaderParam<int> *>(base))->value;
            fwrite(&value, sizeof(value), 1, header_file);
        }
        else if (base->dtype == std::string(DOUBLE))
        {
            double value = (static_cast<HeaderParam<double> *>(base))->value;
            fwrite(&value, sizeof(value), 1, header_file);
        }
        else if (base->dtype == std::string(STRING))
        {
            char *value = (static_cast<HeaderParam<char *> *>(base))->value;
            if (value != NULL)
            {
                int value_length = std::strlen(value);
                fwrite(&value_length, sizeof(int), 1, header_file);
                fwrite(value, sizeof(*value), value_length, header_file);
            }
            else
            {
                int value_length = 0;
                fwrite(&value_length, sizeof(value_length), 1, header_file);
            }
        }
        else if (base->dtype == std::string(NULL_STR))
        {
            int null = 0;
            fwrite(&null, sizeof(null), 1, header_file);
        }
    }

    int header_end_length = std::strlen(HEADER_END);
    fwrite(&header_end_length, sizeof(int), 1, header_file);
    fwrite(HEADER_END, sizeof(char), header_end_length, header_file);
}

int FileTypes::SigprocFile::write_data(unsigned char *data, long length)
{
    fwrite(data, sizeof(char), length, data_file);
    return EXIT_SUCCESS;
}

int FileTypes::SigprocFile::write_data()
{
    std::cerr << "Trying to write: " << data_bytes << "bytes to " << data_file << std::endl;

    // for(unsigned long i=0;i<data_bytes; i++) std::cerr << i << " "<< data[i] << std::endl;

    fwrite(data, sizeof(unsigned char), data_bytes, data_file);
    return EXIT_SUCCESS;
}

int FileTypes::SigprocFile::rewind_to_data_start()
{
    fseek(this->data_file, this->header_bytes, SEEK_SET);
    std::cerr << "ftell:" << ftell(this->data_file) << std::endl;
    return EXIT_SUCCESS;
}
int FileTypes::SigprocFile::rewind_to_file_start()
{
    rewind(this->data_file);
    return EXIT_SUCCESS;
}

size_t FileTypes::SigprocFile::read_num_bytes(size_t nbytes, char *bytes)
{
    size_t count = fread(bytes, nbytes, 1, data_file);
    bytes[nbytes] = '\0';
    return count;
}

int FileTypes::SigprocFile::read_int()
{
    int result;
    size_t count = fread(&result, sizeof(result), 1, data_file);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}

double FileTypes::SigprocFile::read_double()
{
    double result;
    size_t count = fread(&result, sizeof(result), 1, data_file);
    if (count)
        return result;
    else
        return EXIT_FAILURE;
}

size_t FileTypes::SigprocFile::read_num_bytes_to_read()
{
    int nbytes = 4;
    unsigned char bytes[nbytes];
    size_t count = fread(&bytes, nbytes, 1, data_file);
    if (count)
        return (bytes[0] + bytes[1] * 256 + bytes[2] * 65536 + bytes[3] * 16777216);
    else
        return EXIT_FAILURE;
}

double FileTypes::SigprocFile::get_rms()
{
    double sumsq = 0.0;
    double sum = 0.0;
    double mean, meansq;
    for (int loop1 = 0; loop1 < data_bytes; loop1++)
    {
        sumsq += std::pow(this->data[loop1], 2);
        sum += this->data[loop1];
    }
    mean = sum / (double)data_bytes;
    meansq = sumsq / (double)data_bytes;
    float rms = std::sqrt(meansq - mean * mean);
    return rms;
}

double FileTypes::SigprocFile::get_mean()
{
    double sum = 0.0, mean = 0;
    for (int loop1 = 0; loop1 < data_bytes; loop1++)
    {
        sum += this->data[loop1];
    }
    mean = sum / data_bytes;
    return mean;
}
