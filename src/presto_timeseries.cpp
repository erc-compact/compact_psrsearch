#include "presto_timeseries.hpp"
#include <fstream>
#include <string>
#include <stdexcept>
#include <unistd.h> // for getlogin()
#include "sigproc_file.hpp"
#include "constants.hpp"
#include "gen_utils.hpp"
using namespace FileTypes;
/**
 * Writes the header of the PrestoTimeSeries.
 */
void PrestoTimeSeries::write_header(){


    std::string inffn = this->header_file_name;
    if ( this-> header_file != NULL) {
        fclose(this->header_file);
    } 
    std::ofstream ff(inffn);
    if (ff.is_open()) {
        ff << " Data file name without suffix          =  " << this->data_file_name << "\n";
        ff << " Telescope used                         =  " << this->get_value_for_key<const char *>(TELESCOPE_ID) << "\n";
        ff << " Instrument used                        =  " << this->get_value_or_default_for_key<const char *>(MACHINE_ID, "UNKNOWN") << "\n";
        ff << " Object being observed                  =  " << this->get_value_for_key<const char *>(SOURCE_NAME) << "\n";
        ff << " J2000 Right Ascension (hh:mm:ss.ssss)  =  " << this->get_value_for_key<const char *>(SRC_RAJ) << "\n";
        ff << " J2000 Declination     (dd:mm:ss.ssss)  =  " << this->get_value_for_key<const char *>(SRC_DEJ) << "\n";
        ff << " Data observed by                       =  UNKNOWN\n";
        ff << " Epoch of observation (MJD)             =  " << std::fixed << std::setprecision(15) << this->get_value_for_key<double>(TSTART) << "\n";
        ff << " Barycentered?           (1=yes, 0=no)  =  " << this->get_value_or_default_for_key<double>(BARYCENTRIC,0) << "\n";
        ff << " Number of bins in the time series      =  " << this->get_value_for_key<long>(NSAMPLES) << "\n";
        ff << " Width of each time series bin (sec)    =  " << std::fixed << std::setprecision(15) << this->get_value_for_key<double>(TSAMP) << "\n";
        ff << " Any breaks in the data? (1 yes, 0 no)  =  0\n";
        ff << " Orbit removed?          (1=yes, 0=no)  =  " << 0 << "\n";
        ff << " Type of observation (EM band)          =  Radio\n";
        ff << " Dispersion measure (cm-3 pc)           =  " << this->get_value_for_key<double>(REFDM) << "\n";
        ff << " Central freq of low channel (Mhz)      =  " << this->fch1 << "\n";
        ff << " Total bandwidth (Mhz)                  =  " << (this->nchans * this->foff) << "\n";        
        ff << " Number of channels                     =  " << this->nchans << "\n";
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

void read_inf_line_valstr(std::ifstream& infofile, std::string& valstr, const std::string& errdesc) {
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
        std::string sptr = remove_whitespace(line.substr(ii));
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

void PrestoTimeSeries::read_header(){

    std::vector<std::string> keys = {TELESCOPE_ID, MACHINE_ID, SOURCE_NAME, SRC_RAJ, SRC_DEJ, TSTART, BARYCENTRIC, NSAMPLES, TSAMP, REFDM, NCHANS, FOFF};
    std::ifstream infofile(this->header_file_name);
    if (infofile.is_open()) {
        std::string valstr;
        for (auto key : keys) {
            read_inf_line_valstr(infofile, valstr, key);
            HeaderParamBase *header_param = get_header_param(key);
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
        throw std::runtime_error("Error:  Unable to open file '" + this->header_file_name + "' in readinf()\n");
    }


}



