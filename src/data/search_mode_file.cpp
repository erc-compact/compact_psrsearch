#include "data/search_mode_file.hpp"
#include "data/header_params.hpp"
#include "data/constants.hpp"
#include "data/sigproc_file.hpp"
#include "utils/gen_utils.hpp"
#include "utils/sigproc_utils.hpp"
#include "exceptions.hpp"
#include <string>
#include <cstring>
#include <cmath>

using namespace IO;


SearchModeFile* SearchModeFile::createInstance(std::string file_name, std::string mode, std::string file_type){

            SearchModeFile *searchFileObj;

            if(file_type.empty()){
                file_type = guessFileType(file_name);
            }

            if (caseInsensitiveCompare(file_type, "filterbank") || 
                caseInsensitiveCompare(file_type, "sigproc_filterbank") || 
                caseInsensitiveCompare(file_type, "fil")){
                    searchFileObj = new IO::SigprocFile();
            }
            // else if (caseInsensitiveCompare(file_type, "presto_timeseries") || 
            //          caseInsensitiveCompare(file_type, "presto")){
            //     searchFileObj = new PrestoTimeSeries();
            // }
            // else if (caseInsensitiveCompare(file_type, "tim") || 
            //          caseInsensitiveCompare(file_type, "sigproc_timeseries")){
            //     searchFileObj = new SigprocTimeSeries();
            // }
            else{
                throw FileFormatNotRecognised(file_type);
            }

}

std::string SearchModeFile::guessFileType(std::string file_name){
            std::string file_type;
            std::string extension = file_name.substr(file_name.find_last_of(".") + 1);
            if (caseInsensitiveCompare(extension, "fil") || caseInsensitiveCompare(extension, "filterbank") || caseInsensitiveCompare(extension, "sigproc_filterbank")){
                file_type = "filterbank";
            }
            else if (caseInsensitiveCompare(extension, "dat")){
                file_type = "presto_timeseries";
            }
            else{
                throw FileFormatNotRecognised(extension);
            }
            return file_type;
        }



/**
 * @brief Retrieves the header parameter with the specified key.
 *
 * @param key The key of the header parameter to retrieve.
 * @return A pointer to the header parameter if found, otherwise NULL.
 */
HeaderParamBase *SearchModeFile::getHeaderParam(std::string key)
{

    std::map<std::string, HeaderParamBase *>::iterator it = headerParams.find(key);
    if (it != headerParams.end())
    {
        return it->second;
    }
    return NULL
}

void SearchModeFile::removeHeaderParam(std::string key)
{
   
    std::map<std::string, HeaderParamBase *>::iterator it = headerParams.find(key);
    if (it != headerParams.end()) headerParams.erase(it);
    else throw HeaderParamNotFound(key);
    
}

void SearchModeFile::prettyPrint_header()
{
    int max_key_length = 3, max_value_length = 5;

    //iterate through the header params and find the max key and value length
    for (std::map<std::string, HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
    {
        HeaderParamBase *base = it->second;
        max_key_length = std::max(max_key_length, base->getKeyLength());
        max_value_length = std::max(max_value_length, base->getValueLength());
    }
    max_key_length += 2;
    max_value_length += 2;

    std::cout << std::left << std::setw(max_key_length) << "Key" << std::setw(max_value_length) << "Value"
              << "\n";
    std::cout << std::string(max_value_length + max_key_length, '-') << "\n";

    for (std::map<std::string, HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
    {
        HeaderParamBase *header_param = it->second;
        if (header_param != nullptr)
        {
            header_param->prettyPrint(max_key_length, max_value_length);
        }
    }
}

void SearchModeFile::print_header()
{
    for (std::map<std::string, HeaderParamBase *>::iterator it = headerParams.begin(); it != headerParams.end(); ++it)
    {
        HeaderParamBase *base = it->second;
        if (!base->inheader)
            continue;
        std::cerr << base->key << "(" << base->dtype << "): ";
        if (base->dtype == std::string(INT))
        {
            std::cerr << (static_cast<HeaderParam<int> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(LONG))
        {
            std::cerr << (static_cast<HeaderParam<long> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(DOUBLE))
        {
            double value = (static_cast<HeaderParam<double> *>(base))->value;
            if (base->key == std::string(SRC_RAJ))
            {
                std::string str_value;
                sigproc_to_hhmmss(value, str_value);
                std::cerr << str_value << std::endl;
            }
            else if (base->key == std::string(SRC_DEJ))
            {
                std::string str_value;
                sigproc_to_ddmmss(value, str_value);
                std::cerr << str_value << std::endl;
            }
            else
            {
                std::cerr << value << std::endl;
            }
        }
        else if (base->dtype == std::string(STRING))
        {
            if ((static_cast<HeaderParam<char *> *>(base))->value != NULL)
                std::cerr << (static_cast<HeaderParam<char *> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(NULL_STR))
        {
            std::cerr << "NULL" << std::endl;
        }
    }
}

std::size_t SearchModeFile::samplesToBytes(std::size_t nsamples)
{
    int nChans = getValueForKey<int>(NCHANS);
    int nBits = getValueForKey<int>(NBITS);
    int nbytes = nBits / 8;
    return nsamples * nChans * nbytes;
}

std::size_t SearchModeFile::bytesToSamples(std::size_t nBytes)
{
    int nChans = getValueForKey<int>(NCHANS);
    int nBits = getValueForKey<int>(NBITS);
    int numBytesPerSample = nBits / 8;
    return nBytes / nChans / numBytesPerSample;
}

std::size_t SearchModeFile::timeToBytes(std::size_t nsecs)
{
    std::size_t samples = nsecs / getValueForKey<double>(TSAMP);
    return samplesToBytes(samples);
}

std::size_t getTotalDataSize()
{
    return this->nBytesFromDisk;
}

double SearchModeFile::get_mean()
{
    std::size_t nSamps = getValueForKey<long>(NSAMPS);
    double mean = 0.0;
    for (std::size_t i = 0; i < nSamps; ++i)
    {
        mean += data[i];
    }
    return mean / nSamps;
}

double SearchModeFile::get_rms()
{
    long nSamps = getValueForKey<long>(NSAMPS);
    double sumsq = 0.0;
    double sum = 0.0;
    for (std::size_t i = 0; i < nSamps; ++i)
    {
        sumsq += data[i] * data[i];
        sum += data[i];
    }
    double mean = sum / nSamps;
    double rms = std::sqrt(sumsq / nSamps - mean * mean);
    return rms;
}


void SearchModeFile::skip_samples(long nsamples)
{
    skip_bytes(samplesToBytes(nsamples));
}
void SearchModeFile::skip_time(double nsecs)
{
    skip_bytes(timeToBytes(nsecs));
}

void SearchModeFile::goto_sample(int isample)
{
    go_to_byte(samplesToBytes(isample));
}
void SearchModeFile::goto_timestamp(double itime)
{
    goto_byte(timeToBytes(itime));
}

void SearchModeFile::read_nsecs(double start_sec, double nsecs)
{
    std::size_t nbytes = timeToBytes(nsecs);
    std::size_t start_byte = timeToBytes(start_sec);
    readNBytes(start_byte, nbytes);

}

void SearchModeFile::read_nSamps(std::size_t start_sample, std::size_t num_samples)
{
    nbytes = samplesToBytes(num_samples);
    start_byte = samplesToBytes(start_sample);
    readNBytes(start_byte, nbytes);
}

void SearchModeFile::skip_bytes(std::size_t nbytes)
{
    fseek(this->dataFile, bytes_to_skip, SEEK_CUR);
}

void SearchModeFile::goto_byte(std::size_t byte)
{
    fseek(this->dataFile, byte, SEEK_SET);
}


void SearchModeFile::nsamples_in_buffer()
{
    return this->data_buffer->nbytes / this->nChans / this->nBits / 8;
}