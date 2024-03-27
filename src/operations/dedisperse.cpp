#include "operations/dedisperse.hpp"
#include "utils/gen_utils.hpp"
#include "exceptions.hpp"
#include "utils/app_utils.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace OPS;

/* Copied from DEDISP library: https://github1s.com/ewanbarr/dedisp/blob/master/src/kernels.cuh#L56-L81 for easier dedisperse object creation*/
void Dedisperser::populateDMList(std::vector<float>& dmList, float dmStart, float dmEnd, double width, double dmTol, 
					  double tSamp, double f0, double channelBW, int nChans) {


	tSamp *= 1e6;
	double f    = (f0 + ((nChans/2) - 0.5) * channelBW) * 1e-3;
	double tol2 = dmTol*dmTol;
	double a    = 8.3 * channelBW / (f*f*f);
	double a2   = a*a;
	double b2   = a2 * (double)(nChans*nChans / 16.0);
	double c    = (tSamp*tSamp + width*width) * (tol2 - 1.0);
	
	dmList.push_back(dmStart);
	while( dmList.back() < dmEnd ) {
		double prev     = dmList.back();
		double prev2    = prev*prev;
		double k        = c + tol2*a2*prev2;
		double dm = ((b2*prev + sqrt(-a2*b2*prev2 + (a2+b2)*k)) / (a2+b2));
		dmList.push_back(dm);
	}
}

void Dedisperser::populateDMList(std::vector<float>& dmList, const std::string dmFile) {
    std::vector<float> newDMList = generateListFromAsciiRangeFile<float>(dmFile);
   dmList.swap(newDMList);
}


Dedisperser::Dedisperser(std::shared_ptr<IO::SearchModeFile> searchModeFile, unsigned int numGpus, std::vector<float> dmList, 
                            std::string outputDir, std::string outputPrefix, std::string outputSuffix, 
                            std::string outputFormat): Dedisperser(searchModeFile, numGpus, dmList) {


    dedisp_error error = dedisp_create_plan_multi(&plan,
                    searchModeFile->getValueForKey<std::size_t>(NSAMPS),
                    searchModeFile->getValueForKey<float>(TSAMP),
                    searchModeFile->getValueForKey<float>(FCH1),
                    searchModeFile->getValueForKey<float>(FOFF),
                    numGpus);

    ErrorChecker::check_dedisp_error(error,"create_plan_multi");

    this->maxDelaySamples = dedisp_get_max_delay(plan);

    //std::size_t nSamplesOut = searchModeFile->bytesToSamples(nBytesToRead)  - maxDelaySamples;
    this->multiTimeSeries = std::make_unique<IO::MultiTimeSeries>(dmList, outputDir, outputPrefix, outputSuffix, outputFormat);
    this->writeToFile = true;
}

Dedisperser::Dedisperser(std::shared_ptr<IO::SearchModeFile> searchModeFile, unsigned int numGpus, std::vector<float> dmList){
    this->searchModeFile = searchModeFile;
    this->numGpus = numGpus;
    this->dmList = dmList;

    this->writeToFile = false;

    killmask.resize(searchModeFile->getNChans(),1);
    this->multiTimeSeries = nullptr;   

}

void Dedisperser::setDMList(std::vector<float>& dmList)
{
    this->dmList.clear();
    this->dmList.resize(dmList.size());
    std::copy(dmList.begin(), dmList.end(), this->dmList.begin());
    dedisp_error error = dedisp_set_dm_list(plan, &this->dmList[0], this->dmList.size());
    this->maxDelaySamples = dedisp_get_max_delay(plan);
    ErrorChecker::check_dedisp_error(error, "set_dm_list");
}


void Dedisperser::setKillMask(std::vector<int> killmask_in)
{
    killmask.swap(killmask_in);
    dedisp_error error = dedisp_set_killmask(plan, &killmask[0]);
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}

void Dedisperser::setKillMask(std::string fileName)
{
    std::vector<int> newKillMask = generateListFromAsciiMaskFile<int>(fileName, searchModeFile->getNChans());
    killmask.swap(newKillMask);
    dedisp_error error = dedisp_set_killmask(plan, &killmask[0]);
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}





