#pragma once
#define FILREAD "rb"
#define FILWRITE "wb"
#define VIRTUALFIL ""
#define BITS_PER_BYTE 8

const char* const HEADER_START = "HEADER_START";
const char* const HEADER_END = "HEADER_END";
const char* const NCHANS = "nchans";
const char* const NBITS = "nbits";
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
