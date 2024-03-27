#include "data/presto_timeseries.hpp"
#include <fstream>
#include <string>
#include <stdexcept>
#include <unistd.h> // for getlogin()
#include "data/constants.hpp"
#include "utils/gen_utils.hpp"
#include "data/search_mode_file.hpp"
#include <memory>

using namespace IO;

PrestoTimeSeries::PrestoTimeSeries(std::string fileName, std::string mode) : SearchModeFile(fileName, mode){
    this->headerFileName = replaceExtension(fileName, ".inf");

    this->headerFileOpenMode = mode;
    
    if (mode == READ)
    {
        this->readHeader();
        this->nBytesOnDisk = this->nSamps * this->nChans * this->nBits / BITS_PER_BYTE;
        this->nBytesOnRam = this->nBytesOnDisk;
    }
    
   
}

bool PrestoTimeSeries::isHeaderSeparate(){
    return true;
}

void PrestoTimeSeries::readNBytes(std::size_t startByte, std::size_t nBytes) {
    this->nBytesOnDisk = nBytes;
    this->nBytesOnRam = nBytes;
    this->container = std::make_unique<DataBuffer<PRESTO_DAT_TYPE>>(startByte, nBytesOnRam); 
    std::shared_ptr<std::vector<PRESTO_DAT_TYPE>> buffer = this->container->getBuffer<PRESTO_DAT_TYPE>();
    readFromFileAndVerify<PRESTO_DAT_TYPE>(this->dataFile, nBytesOnDisk, buffer->data());
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

    openHeaderFile();
    std::string inffn = this->headerFileName;
    std::ofstream ff(inffn);
    if (ff.is_open()) {
        ff << " Data file name without suffix          =  " << this->dataFileName << "\n";
        ff << " Telescope used                         =  " << this->getValueForKey<const char *>(TELESCOPE_ID) << "\n";
        ff << " Instrument used                        =  " << this->getValueOrDefaultForKey<const char *>(MACHINE_ID, "UNKNOWN") << "\n";
        ff << " Object being observed                  =  " << this->getValueForKey<const char *>(SOURCE_NAME) << "\n";
        ff << " J2000 Right Ascension (hh:mm:ss.ssss)  =  " << this->getValueForKey<const char *>(SRC_RAJ) << "\n";
        ff << " J2000 Declination     (dd:mm:ss.ssss)  =  " << this->getValueForKey<const char *>(SRC_DEJ) << "\n";
        ff << " Data observed by                       =  COMPACT\n";
        ff << " Epoch of observation (MJD)             =  " << std::fixed << std::setprecision(15) << this->getValueForKey<double>(TSTART) << "\n";
        ff << " Barycentered?           (1=yes, 0=no)  =  " << this->getValueOrDefaultForKey<double>(BARYCENTRIC,0) << "\n";
        ff << " Number of bins in the time series      =  " << this->getValueForKey<long>(NSAMPLES) << "\n";
        ff << " Width of each time series bin (sec)    =  " << std::fixed << std::setprecision(15) << this->getValueForKey<double>(TSAMP) << "\n";
        ff << " Any breaks in the data? (1 yes, 0 no)  =  0\n";
        ff << " Orbit removed?          (1=yes, 0=no)  =  " << 0 << "\n";
        ff << " Type of observation (EM band)          =  Radio\n";
        ff << " Dispersion measure (cm-3 pc)           =  " << this->getValueForKey<double>(REFDM) << "\n";
        ff << " Central freq of low channel (Mhz)      =  " << this->fch1 << "\n";
        ff << " Total bandwidth (Mhz)                  =  " << (this->nChans * this->foff) << "\n";        
        ff << " Number of channels                     =  " << this->nChans << "\n";
        ff << " Channel bandwidth (Mhz)                =  " << this->foff << "\n";
        ff << " Data analyzed by                       =  " << getlogin() << "\n";
        ff << " Any additional notes:\n"
           << "    File written by COMPACT's pulsar search package\n";
    }
    ff.close();

}



/**
 * Copied from presto:ioinf.c and modified to C++ style
*/

void readInfLineValStr(std::ifstream& infofile, std::string& valstr, const std::string& errdesc) {
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



