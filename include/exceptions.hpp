#pragma once
#include <iostream>
#include <sstream>
#include "dedisp.h"
#include "cuda.h"
#include "cuda_runtime.h"
#include "cufft.h"
#include <fstream>
#include <execinfo.h>


class CustomException : public std::exception
{
protected:
    std::string message;
public:
    CustomException() { }
    CustomException(std::string message)
    {
        this->message = message;
    }

    virtual const char *what() const throw()
    {
        return message.c_str();
    }
};

class FileFormatNotRecognised : public CustomException
{
public:
    FileFormatNotRecognised(std::string file_name) 
    {
        std::string message = "File format not recognised for file: " + file_name;
        this->message = message;
    }
};

class HeaderParamNotFound : public CustomException
{
  public:
    HeaderParamNotFound(std::string key) 
    {
        std::string message = "Header does not have key: " + key;
        this->message = message;
    }

};

class FileIOError : public CustomException
{
  public:
    FileIOError(std::size_t required, std::size_t got, std::string operation) 
    {
        std::stringstream message;
        message << "Error in file I/O. Could not " << operation << " " << required << " bytes. Got only " << got << " bytes.";
        this->message = message.str();
    }
};

class FunctionalityNotImplemented : public CustomException
{
  public:
    FunctionalityNotImplemented(std::string msg) 
    {
        std::string message = "Functionality not implemented: " + msg;
        this->message = message;
    }
};

class InvalidInputs : public CustomException
{
  public:
    InvalidInputs(std::string msg) 
    {
        std::string message = "Invalid inputs: " + msg;
        this->message = message;
    }
};




/**
 * The following class is heavily borrowed from Peasoup. Credits to the original writers. 
 * 
*/
class ErrorChecker {
public:
  static void check_dedisp_error(dedisp_error error, std::string function_name)
  {
    if (error != DEDISP_NO_ERROR) {
      std::stringstream error_msg;
      error_msg << function_name << " failed with DEDISP error: "<< dedisp_get_error_string(error)	<< std::endl;
      throw std::runtime_error(error_msg.str());
    }
  }
  
  static void checkFileError(std::ifstream& infile, std::string fileName){
    if(!infile.good()) {
      std::stringstream error_msg;
      error_msg << "File "<< fileName << " could not be opened: ";
      
      if ( (infile.rdstate() & std::ifstream::failbit ) != 0 )
	error_msg << "Logical error on i/o operation" << std::endl;

      if ( (infile.rdstate() & std::ifstream::badbit ) != 0 )
	error_msg << "Read/writing error on i/o operation" << std::endl;

      if ( (infile.rdstate() & std::ifstream::eofbit ) != 0 )
	error_msg << "End-of-File reached on input operation" << std::endl;

      throw std::runtime_error(error_msg.str());
    }
  }

  static void checkFileError(std::ofstream& infile, std::string fileName){
    if(!infile.good()) {
      std::stringstream error_msg;
      error_msg << "File "<< fileName << " could not be opened: ";

      if ( (infile.rdstate() & std::ifstream::failbit ) != 0 )
        error_msg << "Logical error on i/o operation" << std::endl;

      if ( (infile.rdstate() & std::ifstream::badbit ) != 0 )
        error_msg << "Read/writing error on i/o operation" << std::endl;

      if ( (infile.rdstate() & std::ifstream::eofbit ) != 0 )
        error_msg << "End-of-File reached on input operation" << std::endl;

      throw std::runtime_error(error_msg.str());
    }
  }

  static void check_cuda_error(std::string msg="Unspecified location"){
    cudaDeviceSynchronize();
    cudaError_t error = cudaGetLastError();
    if (error!=cudaSuccess){
      std::stringstream error_msg;
      error_msg << "CUDA failed with error: "
                << cudaGetErrorString(error) << std::endl 
		<< "Additional: " << msg << std::endl;
      throw std::runtime_error(error_msg.str());
    }
  }

  /*
  static void check_cuda_error(cudaError_t error){
    check_cuda_error(error,"");
  }
  */

  static void throw_error(std::string msg){
    throw std::runtime_error(msg.c_str());
  }

  
  static void check_cufft_error(cufftResult error){
    if (error!=CUFFT_SUCCESS){
      std::stringstream error_msg;
      error_msg << "cuFFT failed with error: ";
      switch (error)
	{
	case CUFFT_INVALID_PLAN:
	  error_msg <<  "CUFFT_INVALID_PLAN";
	  break;

	case CUFFT_ALLOC_FAILED:
	  error_msg <<  "CUFFT_ALLOC_FAILED";
	  break;

	case CUFFT_INVALID_TYPE:
	  error_msg <<  "CUFFT_INVALID_TYPE";
	  break;

	case CUFFT_INVALID_VALUE:
	  error_msg <<  "CUFFT_INVALID_VALUE";
	  break;

	case CUFFT_INTERNAL_ERROR:
	  error_msg <<  "CUFFT_INTERNAL_ERROR";
	  break;

	case CUFFT_EXEC_FAILED:
	  error_msg <<  "CUFFT_EXEC_FAILED";
	  break;
	  
	case CUFFT_SETUP_FAILED:
	  error_msg <<  "CUFFT_SETUP_FAILED";
	  break;

	case CUFFT_INVALID_SIZE:
	  error_msg <<  "CUFFT_INVALID_SIZE";
	  break;

	case CUFFT_UNALIGNED_DATA:
	  error_msg <<  "CUFFT_UNALIGNED_DATA";
	  break;

	default:
	  error_msg <<  "<unknown>";
	}
      error_msg << std::endl;
      print_stack_trace(20);
      throw std::runtime_error(error_msg.str());
    }
  }

  static void print_stack_trace(unsigned int max_depth){
    int trace_depth;    
    void *buffer[max_depth];
    char **strings;

    trace_depth = backtrace(buffer, max_depth); 
    strings = backtrace_symbols(buffer, trace_depth);
    if (strings == NULL) {
      std::cerr << "Stack trace failed" << std::endl;
    } else {
      for (int jj = 0; jj < trace_depth; jj++)
	std::cerr << strings[jj] << std::endl;
      free(strings);
    }
  }
};

