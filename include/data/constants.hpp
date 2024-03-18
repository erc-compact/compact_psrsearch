#pragma once
#include <string>
#define READ "rb"
#define WRITE "wb"
#define VIRTUALFIL ""
#define BITS_PER_BYTE 8

const std::string HEADER_START = "HEADER_START";
const std::string HEADER_END = "HEADER_END";
const std::string NCHANS = "nChans";
const std::string NBITS = "nBits";
const std::string NIFS = "nifs";
const std::string TSAMP = "tsamp";
const std::string TSTART = "tstart";
const std::string SRC_RAJ = "src_raj";
const std::string SRC_DEJ = "src_dej";
const std::string SOURCE_NAME = "source_name";
const std::string MACHINE_ID = "machine_id";
const std::string TELESCOPE_ID = "telescope_id";
const std::string BARYCENTRIC= "barycentric";
const std::string REFDM = "refdm";


const std::string NSAMPLES = "nsamples";
const std::string NSAMPS = "nsamples";
const std::string TOBS = "tobs";
const std::string FCH1 = "fch1";
const std::string FOFF = "foff";
const std::string CHANNELBW = "foff";


const std::string HEADER_KEYS_FILE ="resources/header_keys.dat";
const std::string INT = "int";
const std::string DOUBLE = "double";
const std::string STRING = "string";
const std::string LONG = "long";
const std::string NULL_STR = "null";


const std::string BYTES = "bytes";
const std::string SAMPLES = "samples";
const std::string SECONDS = "seconds";

typedef unsigned char BYTE;
typedef unsigned char SIGPROC_FILTERBANK_8_BIT_TYPE;
typedef unsigned short SIGPROC_FILTERBANK_16_BIT_TYPE;
typedef float SIGPROC_FILTERBANK_32_BIT_TYPE;
typedef float SIGPROC_TIMESERIES_TYPE;
typedef float PRESTO_TIMESERIES_TYPE;
typedef unsigned char DEDISP_BYTE;                  
typedef float DEDISP_OUTPUT_TYPE;
typedef unsigned long  DEDISP_SIZE;
typedef int DEDISP_BOOL;


