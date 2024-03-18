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


SearchModeFile* SearchModeFile::createInstance(std::string fileName, std::string mode, std::string fileType){

            SearchModeFile *searchFileObj;

            if(fileType.empty()){
                fileType = guessFileType(fileName);
            }

            if (caseInsensitiveCompare(fileType, "filterbank") || 
                caseInsensitiveCompare(fileType, "sigproc_filterbank") || 
                caseInsensitiveCompare(fileType, "fil")){
                    searchFileObj = new SigprocFile(fileName, mode);
            }
            else if (caseInsensitiveCompare(fileType, "presto_timeseries") || 
                     caseInsensitiveCompare(fileType, "presto")){
                searchFileObj = new PrestoTimeSeries(fileName, mode);
            }
            else if (caseInsensitiveCompare(fileType, "tim") || 
                     caseInsensitiveCompare(fileType, "sigproc_timeseries")){
                searchFileObj = new SigprocTimeSeries(fileName, mode);
            }
            else{
                throw FileFormatNotRecognised(fileType);
            }

}

std::string SearchModeFile::guessFileType(std::string fileName){
            std::string fileType;
            std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
            if (caseInsensitiveCompare(extension, "fil") || caseInsensitiveCompare(extension, "filterbank") || caseInsensitiveCompare(extension, "sigproc_filterbank")){
                fileType = "filterbank";
            }
            else if (caseInsensitiveCompare(extension, "dat")){
                fileType = "presto_timeseries";
            }
            else{
                throw FileFormatNotRecognised(extension);
            }
            return fileType;
        }



/**
 * @brief Retrieves the header parameter with the specified key.
 *
 * @param key The key of the header parameter to retrieve.
 * @return A pointer to the header parameter if found, otherwise NULL.
 */
HeaderParamBase* SearchModeFile::getHeaderParam(std::string key) {

    std::map<std::string, HeaderParamBase *>::iterator it = headerParams.find(key);
    if (it != headerParams.end()) return it->second;
    return nullptr;
}

void SearchModeFile::removeHeaderParam(std::string key) {
   
    std::map<std::string, HeaderParamBase *>::iterator it = headerParams.find(key);
    if (it != headerParams.end()) headerParams.erase(it);
    else throw HeaderParamNotFound(key);
    
}

void SearchModeFile::prettyPrintHeader() {
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

void SearchModeFile::printHeader() {
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

std::size_t SearchModeFile::samplesToBytes(std::size_t nsamples) {
    int nChans = getValueForKey<int>(NCHANS);
    int nBits = getValueForKey<int>(NBITS);
    int nbytes = nBits / 8;
    return nsamples * nChans * nbytes;
}

std::size_t SearchModeFile::bytesToSamples(std::size_t nBytes) {
    int nChans = getValueForKey<int>(NCHANS);
    int nBits = getValueForKey<int>(NBITS);
    int numBytesPerSample = nBits / 8;
    return nBytes / nChans / numBytesPerSample;
}

std::size_t SearchModeFile::timeToBytes(std::size_t nsecs) {
    std::size_t samples = nsecs / getValueForKey<double>(TSAMP);
    return samplesToBytes(samples);
}

std::size_t SearchModeFile::getTotalDataSize() {
    return this->nBytesOnDisk;
}

double SearchModeFile::getMean() {
    std::size_t nSamps = getValueForKey<long>(NSAMPS);
    double mean = 0.0;
    for (std::size_t i = 0; i < nSamps; ++i)
    {
        mean += (*this->container)[i];
    }
    return mean / nSamps;
}

double SearchModeFile::getRMS() {
    long nSamps = getValueForKey<long>(NSAMPS);
    double sumsq = 0.0;
    double sum = 0.0;
    for (std::size_t i = 0; i < nSamps; ++i)
    {   
        float data = (*this->container)[i];
        sumsq += (data * data);
        sum += data;
    }
    double mean = sum / nSamps;
    double rms = std::sqrt(sumsq / nSamps - mean * mean);
    return rms;
}


void SearchModeFile::skipSamples(std::size_t nsamples) {
    skipBytes(samplesToBytes(nsamples));
}

void SearchModeFile::skipTime(double nsecs) {
    skipBytes(timeToBytes(nsecs));
}

void SearchModeFile::gotoSample(std::size_t isample) {
    goToByte(samplesToBytes(isample));
}

void SearchModeFile::gotoTimestamp(double itime) {
    goToByte(timeToBytes(itime));
}

void SearchModeFile::readNSecs(double startSec, double nSecs) {
    std::size_t nBytes = timeToBytes(nSecs);
    std::size_t startByte = timeToBytes(startSec);
    readNBytes(startByte, nBytes);

}

void SearchModeFile::readNSamps(std::size_t startSample, std::size_t numSamples) {
    std::size_t  nBytes = samplesToBytes(numSamples);
    std::size_t startByte = samplesToBytes(startSample);
    readNBytes(startByte, nBytes);
}

void SearchModeFile::skipBytes(std::size_t nBytes) {
    fseek(this->dataFile, nBytes, SEEK_CUR);
}

void SearchModeFile::goToByte(std::size_t byte) {
    fseek(this->dataFile, byte, SEEK_SET);
}


void SearchModeFile::nSamplesInBuffer() {
    return this->container->nBytes / this->nChans / this->nBits / BITS_PER_BYTE;
}