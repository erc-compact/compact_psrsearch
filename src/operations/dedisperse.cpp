#include "operations/dedisperse.hpp"

void Dedisperser::setDmList(float *dmList, unsigned int size)
{
    this->dmList.resize(size);
    std::copy(dmList, dmList + ndms, this->dmList.begin());
    dedisp_error error = dedisp_set_dm_list(plan, &this->dmList[0], this->dmList.size());
    this->maxDelaySamples = dedisp_get_max_delay(plan);

    ErrorChecker::check_dedisp_error(error, "set_dm_list");
}

void Dedisperser::generateDmList(std::string dm_file)
{
    dm_list.swap(generateListFromAsciiRangeFile<float>(dm_file));
}

void Dedisperser::generateDmList(float dm_start, float dm_end, float width, float tolerance)
{
    dedisp_error error = dedisp_generate_dm_list(plan, dm_start, dm_end, width, tolerance);
    ErrorChecker::check_dedisp_error(error, "generateDmList");

    dm_list.resize(dedisp_get_dm_count(plan));

    const float *plan_dm_list = dedisp_get_dm_list(plan);

    std::copy(plan_dm_list, plan_dm_list + dm_list.size(), dm_list.begin());
}

void Dedisperser::setKillMask(std::vector<int> killmask_in)
{
    killmask.swap(killmask_in);
    dedisp_error error = dedisp_set_killmask(plan, &killmask[0]);
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}

void Dedisperser::setKillMask(std::string fileName)
{
    killmask.swap(generateListFromAsciiMaskFile<DEDISP_BOOL>(fileName, searchModeFile->getNchans()));
    dedisp_error error = dedisp_set_killmask(plan, &killmask[0]);
    ErrorChecker::check_dedisp_error(error, "set_killmask");
}

void Dedisperser::dedisperse(DEDISP_BYTE* input_data, DEDISP_SIZE input_size, DEDISP_OUTPUT_TYPE* out_data){
      dedisp_error error = dedisp_execute(plan,
            buffer,
            output,  
            input_size, 
            out_data,
            32, // Float output
            (unsigned) 0);
        ErrorChecker::check_dedisp_error(error,"execute");
}



