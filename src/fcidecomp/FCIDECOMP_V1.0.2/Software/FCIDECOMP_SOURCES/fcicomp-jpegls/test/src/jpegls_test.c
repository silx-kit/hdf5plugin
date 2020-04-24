// $Id: jpegls_test.c 823 2017-09-05 14:30:55Z delaunay $
// =============================================================
//
// PROJECT : FCICOMP
//
// AUTHOR : THALES Services
//
// Copyright 2015 EUMETSAT
//
// =============================================================
// HISTORY :
//
// VERSION:1.0.2:NCR:FCICOMP-12:04/09/2017:Remove the signal handler
// VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
//
// END-HISTORY
// =============================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fcicomp_options.h"

static const char* JPEG_COMPRESS_NOMINAL_TEST 		= "jpeglsCompressNominal";
static const char* JPEG_COMPRESS_NOISE_TEST 		= "jpeglsCompressNoise";
static const char* JPEG_DECOMPRESS_NOISE_TEST 		= "jpeglsDecompressNoise";
static const char* JPEG_DECOMPRESS_NOMINAL_TEST 	= "jpeglsDecompressNominal";
static const char* JPEG_COMPRESS_ERROR_CASE			= "jpeglsCompressErrorCase";
static const char* JPEG_READ_HEADER_ERROR_CASE		= "jpeglsReadHeaderErrorCase";
static const char* JPEG_DECOMPRESS_ERROR_CASE		= "jpeglsDecompressErrorCase";

// Declare the tests
// Compress nominal
int jpeglsCompressNominal(int argc, char* argv[]);
// Decompress nominal
int jpeglsDecompressNominal(int argc, char* argv[]);
// Compress error case
int jpeglsCompressErrorCase(int argc, char* argv[]);
// Read header error case
int jpeglsReadHeaderErrorCase(void);
// Decompress error case
int jpeglsDecompressErrorCase(int argc, char* argv[]);

// Main test function
int main(int argc, char* argv[])
{
	int result = EXIT_FAILURE;

    // Get the test name
    char * testName = argv[FCI_ONE];

    // Call the test functions
    if (strcmp(testName, JPEG_COMPRESS_NOMINAL_TEST) == 0) {
        // launch the test
    	result = jpeglsCompressNominal(argc-1, argv+1);
    }
    if (strcmp(testName, JPEG_COMPRESS_NOISE_TEST) == 0) {
        // launch the test
    	// use the same program as for the jpeglsCompressNominal test
    	result = jpeglsCompressNominal(argc-1, argv+1);
    }
    if (strcmp(testName, JPEG_DECOMPRESS_NOISE_TEST) == 0) {
        // launch the test
    	// use the same program as for the jpeglsDecompressNominal test
    	result = jpeglsDecompressNominal(argc-1, argv+1);
    }
	if (strcmp(testName, JPEG_DECOMPRESS_NOMINAL_TEST) == 0) {
        // launch the test
		result = jpeglsDecompressNominal(argc-1, argv+1);
    }
	if (strcmp(testName, JPEG_COMPRESS_ERROR_CASE) == 0) {
        // launch the test
		result = jpeglsCompressErrorCase(argc-1, argv+1);
    }
	if (strcmp(testName, JPEG_READ_HEADER_ERROR_CASE) == 0) {
        // launch the test
		result = jpeglsReadHeaderErrorCase();
    }
	if (strcmp(testName, JPEG_DECOMPRESS_ERROR_CASE) == 0) {
        // launch the test
		result = jpeglsDecompressErrorCase(argc-1, argv+1);
    }

    return result;
}
