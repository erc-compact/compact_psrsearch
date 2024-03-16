#pragma once
#define READ "rb"
#define WRITE "wb"
#define VIRTUALFIL ""
#define BITS_PER_BYTE 8

const char* const HEADER_START = "HEADER_START";
const char* const HEADER_END = "HEADER_END";
const char* const NCHANS = "nChans";
const char* const NBITS = "nBits";
const char* const NIFS = "nifs";
const char* const TSAMP = "tsamp";
const char* const TSTART = "tstart";
const char* const SRC_RAJ = "src_raj";
const char* const SRC_DEJ = "src_dej";
const char* const SOURCE_NAME = "source_name";
const char* const MACHINE_ID = "machine_id";
const char* const TELESCOPE_ID = "telescope_id";
const char* const BARYCENTRIC= "barycentric";
const char* const REFDM = "refdm";


const char* const NSAMPLES = "nsamples";
const char* const NSAMPS = "nsamples";
const char* const TOBS = "tobs";
const char* const FCH1 = "fch1";
const char* const FOFF = "foff";
const char* const CHANNELBW = "foff";


const char* const HEADER_KEYS_FILE ="resources/header_keys.dat";
const char* const INT = "int";
const char* const DOUBLE = "double";
const char* const STRING = "string";
const char* const LONG = "long";
const char* const NULL_STR = "null";


const char* const BYTES = "bytes";
const char* const SAMPLES = "samples";
const char* const SECONDS = "seconds";


typedef unsigned char SIGPROC_FILTERBANK_8_BIT_TYPE;
typedef unsigned short SIGPROC_FILTERBANK_16_BIT_TYPE;
typedef float SIGPROC_FILTERBANK_32_BIT_TYPE;
typedef float SIGPROC_TIMESERIES_TYPE;
typedef float PRESTO_TIMESERIES_TYPE;
typedef unsigned char DEDISP_BYTE;                  
typedef float DEDISP_OUTPUT_TYPE;
typedef unsigned long  DEDISP_SIZE;
typedef int DEDISP_BOOL;


