#include "data/presto_timeseries.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unistd.h> // for getlogin()
#include "data/constants.hpp"
#include "utils/gen_utils.hpp"
#include "data/search_mode_file.hpp"
#include "utils/sigproc_utils.hpp"
#include <memory>
#include <pwd.h>

using namespace IO;

PrestoTimeSeries::PrestoTimeSeries(std::string fileName, std::string mode) : SearchModeFile(fileName, mode){
    this->headerFileName = replaceExtension(fileName, "inf");

    this->headerFileOpenMode = mode;
    
    if (mode == READ)
    {
        this->readHeader();
        this->nBytesOnDisk = this->nSamps * this->nChans * this->nBits / BITS_PER_BYTE;
        this->nBytesOnRam = this->nBytesOnDisk;
    }

  
}

void PrestoTimeSeries::copyHeaderFrom(std::shared_ptr<SearchModeFile> other){
    SearchModeFile::copyHeaderFrom(other);
    this->updateHeaderValue<int>(NCHANS, 1);
    this->tsamp = other->getValueForKey<float>(TSAMP);
    this->fch1 = other->getValueForKey<float>(FCH1);
    this->foff = other->getValueForKey<float>(FOFF);
    this->nBits = other->getValueForKey<int>(NBITS);
    this->nSamps = other->getValueForKey<long>(NSAMPLES);
    
    this->nChans = 1;



}

bool PrestoTimeSeries::isHeaderSeparate(){
    return true;
}

void PrestoTimeSeries::readNBytes(std::size_t startByte, std::size_t nBytes) {
    this->nBytesOnDisk = nBytes;
    this->nBytesOnRam = nBytes;
    this->container = std::make_unique<DataBuffer<PRESTO_DAT_TYPE>>(startByte, nBytesOnRam); 
    std::shared_ptr<std::vector<PRESTO_DAT_TYPE>> busser = this->container->getBuffer<PRESTO_DAT_TYPE>();
    readFromFileAndVerify<PRESTO_DAT_TYPE>(this->dataFile, nBytesOnDisk, busser->data());
}

void PrestoTimeSeries::writeNBytes() {
    this->nBytesOnRam = this->container->getNBytes();
    this->nBytesOnDisk = this->container->getNBytes();
    size_t inBufferSize = this->container->getNElements();
    std::shared_ptr<std::vector<PRESTO_DAT_TYPE>>  inBuffer = this->container->getBuffer<PRESTO_DAT_TYPE>();
    writeToFileAndVerify<PRESTO_DAT_TYPE>(this->dataFile, inBufferSize, inBuffer->data());
}

void PrestoTimeSeries::readAllData(){}
void PrestoTimeSeries::writeAllData(){}



/**
 * Writes the header of the PrestoTimeSeries.
 */
void PrestoTimeSeries::writeHeader(){

    std::string ra,dec;
    std::string login = getpwuid(getuid())->pw_name;
    sigproc_to_hhmmss(this->getValueForKey<float>(SRC_RAJ), ra);
    sigproc_to_ddmmss(this->getValueForKey<float>(SRC_DEJ), dec);
    openHeaderFile();
    prettyPrintHeader();
    std::stringstream ss;
    ss << " Data file name without suffix          =  " << this->dataFileName << "\n";
    ss << " Telescope used                         =  " << this->getValueForKey<int>(TELESCOPE_ID) << "\n";
    ss << " Instrument used                        =  " << this->getValueOrDefaultForKey<int>(MACHINE_ID, -1) << "\n";
    ss << " Object being observed                  =  " << "J1818-1422" << "\n";
    ss << " J2000 Right Ascension (hh:mm:ss.ssss)  =  " << ra << "\n";
    ss << " J2000 Declination     (dd:mm:ss.ssss)  =  " << dec << "\n";
    ss << " Data observed by                       =  COMPACT\n";
    ss << " Epoch of observation (MJD)             =  " << std::fixed << std::setprecision(15) << this->getValueForKey<float>(TSTART) << "\n";
    ss << " Barycentered?           (1=yes, 0=no)  =  " << this->getValueOrDefaultForKey<int>(BARYCENTRIC,0) << "\n";
    ss << " Number of bins in the time series      =  " << this->getValueForKey<long>(NSAMPLES) << "\n";
    ss << " Width of each time series bin (sec)    =  " << std::fixed << std::setprecision(15) << this->getValueForKey<float>(TSAMP) << "\n";
    ss << " Any breaks in the data? (1 yes, 0 no)  =  0\n";
    ss << " Orbit removed?          (1=yes, 0=no)  =  " << 0 << "\n";
    ss << " Type of observation (EM band)          =  Radio\n";
    ss << " Dispersion measure (cm-3 pc)           =  " << this->getValueForKey<float>(REFDM) << "\n";
    ss << " Central freq of low channel (Mhz)      =  " << this->getValueForKey<float>(FCH1) << "\n";
    ss << " Total bandwidth (Mhz)                  =  " << std::fixed << std::setprecision(6) << this->getValueForKey<float>(BW) << "\n";        
    ss << " Number of channels                     =  " << this->nChans << "\n";
    ss << " Channel bandwidth (Mhz)                =  " << this->getValueForKey<float>(FOFF) << "\n";
    ss << " Data analyzed by                       =  " << login << "\n";
    ss << " Any additional notes:\n";
    ss    << "    File written by COMPACT's pulsar search package\n";
    writeToFileAndVerify<const char>(this->headerFile, ss.str().size(), ss.str().c_str());


}



/**
 * Copied from presto:ioinf.c and modified to C++ style
*/

void PrestoTimeSeries::readInfLineValStr(std::ifstream& infofile, std::string& valstr, const std::string& errdesc) {
    std::string line;
    std::getline(infofile, line);
    if (!line.empty() && line[0] != '\n') {
        std::string::size_type ii;
        // Check to see if this is a "standard" .inf line
        // which has an '=' in character 40
        if (line.length() >= 40 && line[40] == '=') {
            ii = 41;
        } else {
            // Else, look for the first '=' back from the end of the line
            ii = line.rfind('=');
            if (ii == std::string::npos) {
                throw std::runtime_error("Error:  no '=' to separate key/val while looking for '" + errdesc + "' in readinf()\n");
            }
            ii++;
        }
        std::string sptr = removeWhiteSpace(line.substr(ii));
        if (!sptr.empty()) {
            if ((errdesc == "data->name" && sptr.length() > 199) ||
                (errdesc == "data->telescope" && sptr.length() > 39) ||
                (errdesc == "data->band" && sptr.length() > 39) ||
                (errdesc != "data->name" && sptr.length() > 99)) {
                throw std::runtime_error("Error:  value string is too long (" + std::to_string(sptr.length()) + " char) while looking for '" + errdesc + "' in readinf()\n");
            }
            valstr = sptr;
        } else {
            valstr = "Unknown";
        }
        return;
    } else {
        if (infofile.eof()) {
            throw std::runtime_error("Error:  end-of-file while looking for '" + errdesc + "' in readinf()\n");
        } else {
            throw std::runtime_error("Error:  found blank line while looking for '" + errdesc + "' in readinf()\n");
        }
    }
    
}

/**
 * The only way to read the inf file (since it is written in a very human friendly format) is to read it line by line in the order that it was saved.
 * The order below cannot be changed. 
*/

void PrestoTimeSeries::readHeader(){

    std::vector<std::string> keys = {TELESCOPE_ID, MACHINE_ID, SOURCE_NAME, SRC_RAJ, SRC_DEJ, TSTART, BARYCENTRIC, NSAMPLES, TSAMP, REFDM, NCHANS, FOFF};
    std::ifstream infofile(this->headerFileName);
    if (infofile.is_open()) {
        std::string valstr;
        for (auto key : keys) {
            readInfLineValStr(infofile, valstr, key);
            HeaderParamBase *header_param = getHeaderParam(key);
            std::string dtype = header_param->dtype;
            header_param->inheader = true;
            if (dtype == std::string(INT)){
                static_cast<HeaderParam<int> *>(header_param)->value = std::stoi(valstr);
            }
            else if (dtype == std::string(DOUBLE)){
                static_cast<HeaderParam<double> *>(header_param)->value = std::stod(valstr);
            }
            else if (dtype == std::string(STRING)){

                static_cast<HeaderParam<const char *> *>(header_param)->value = valstr.c_str();
            }
            
        }
        infofile.close();
    } else {
        throw std::runtime_error("Error:  Unable to open file '" + this->headerFileName + "' in readinf()\n");
    }


}



