#include "operations/dedisperse.hpp"
#include "utils/gen_utils.hpp"
#include "exceptions.hpp"
#include "utils/app_utils.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace OPS;

/* Copied from DEDISP library: https://github1s.com/ewanbarr/dedisp/blob/master/src/kernels.cuh#L56-L81 for easier dedisperse object creation*/
void Dedisperser::populateDMList(std::shared_ptr<std::vector<float>> dmList, float dmStart, float dmEnd, double width, double dmTol, 
					  double tSamp, double f0, double channelBW, int nChans) {


	tSamp *= 1e6;
	double f    = (f0 + ((nChans/2) - 0.5) * channelBW) * 1e-3;
	double tol2 = dmTol*dmTol;
	double a    = 8.3 * channelBW / (f*f*f);
	double a2   = a*a;
	double b2   = a2 * (double)(nChans*nChans / 16.0);
	double c    = (tSamp*tSamp + width*width) * (tol2 - 1.0);
	
	dmList->push_back(dmStart);
	while( dmList->back() < dmEnd ) {
		double prev     = dmList->back();
		double prev2    = prev*prev;
		double k        = c + tol2*a2*prev2;
		double dm = ((b2*prev + sqrt(-a2*b2*prev2 + (a2+b2)*k)) / (a2+b2));
		dmList->push_back(dm);
	}

}

void Dedisperser::populateDMList(std::shared_ptr<std::vector<float>> dmList, const std::string dmFile) {
   std::shared_ptr<std::vector<float>> newDMList = generateListFromAsciiRangeFile<float>(dmFile);
   dmList->swap(*newDMList);
}




Dedisperser::Dedisperser(std::shared_ptr<IO::SearchModeFile> searchModeFile, unsigned int numGpus, std::shared_ptr<std::vector<float>> dmList, bool writeToFile, std::size_t gulpNSamples){

    this->searchModeFile = searchModeFile;
    this->numGpus = numGpus;
    this->writeToFile = writeToFile;

    if(gulpNSamples == 0) {
        gulping = false;
        this->gulpNSamples = searchModeFile->getValueForKey<std::size_t>(NSAMPLES);
    }
    else if (gulpNSamples < searchModeFile->getValueForKey<std::size_t>(NSAMPLES)) {
        gulping= true; 
        this->gulpNSamples = gulpNSamples;  
    }
    else if (gulpNSamples == searchModeFile->getValueForKey<std::size_t>(NSAMPLES)) {
        gulping = false;
        this->gulpNSamples = gulpNSamples;
    }

    else {
        throw InvalidInputs("NSAMPLES to gulp cannot be greater than NSAMPLES");
    }

    dedisp_error error = dedisp_create_plan(&this->plan,
                    searchModeFile->getValueForKey<int>(NCHANS),
                    searchModeFile->getValueForKey<float>(TSAMP),
                    searchModeFile->getValueForKey<float>(FCH1),
                    searchModeFile->getValueForKey<float>(FOFF));
    ErrorChecker::check_dedisp_error(error,"create_plan_multi");
    this->setDMList(dmList);
    this->maxDelaySamples = dedisp_get_max_delay(plan);
    this->multiTimeSeries = std::make_unique<IO::MultiTimeSeries>(dmList, this->gulpNSamples, searchModeFile->getValueForKey<std::size_t>(NSAMPLES), writeToFile);
    this->killmask = std::make_shared<std::vector<DEDISP_BOOL>>(searchModeFile->getNChans(),1);
}

void Dedisperser::setOutputOptions(std::string outputDir, std::string outputPrefix, std::string outputSuffix, std::string outputFormat, std::shared_ptr<IO::SearchModeFile> searchModeFile){
    if(!this->writeToFile) throw new InvalidInputs("Cannot set output options when writeToFile is false");
    this->multiTimeSeries->initOutputOptions(outputDir, outputPrefix, outputSuffix, outputFormat, searchModeFile);
}

void Dedisperser::setDMList(std::shared_ptr<std::vector<float>> dmList)
{

    if (this->dmList != nullptr) {
        this->dmList->clear();
        this->dmList->reserve(dmList->size());
    }
    else {
        this->dmList = std::make_shared<std::vector<float>>(dmList->size());
    }
    std::copy(dmList->begin(), dmList->end(), this->dmList->begin());

    dedisp_error error = dedisp_set_dm_list(this->plan, this->dmList->data(), this->dmList->size());
    this->maxDelaySamples = dedisp_get_max_delay(plan);
    ErrorChecker::check_dedisp_error(error, "set_dm_list");
}

void Dedisperser::setKillMask(std::shared_ptr<std::vector<int>> killmask_in)
{
    killmask->swap(*killmask_in);
    dedisp_error error = dedisp_set_killmask(plan, &killmask->at(0));
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}

void Dedisperser::setKillMask(std::string fileName)
{
    std::shared_ptr<std::vector<int>> newKillMask = generateListFromAsciiMaskFile<int>(fileName, searchModeFile->getNChans());
    killmask->swap(*newKillMask);
    dedisp_error error = dedisp_set_killmask(plan, &killmask->at(0));
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}





