// $Id: H5Zjpegls_test.c 778 2016-03-09 07:56:29Z delaunay $
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
// VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
//
// END-HISTORY
// =============================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fcicomp_options.h"

static const char* H5_ZJPEGLS_DECODE_INEFFECTIVE = "H5ZjpeglsDecodeIneffective";
static const char* H5_ZJPEGLS_DECODE_RGBA_NOMINAL = "H5ZjpeglsDecodeRGBANominal";
static const char* H5_ZJPEGLS_DECODE_NOMINAL = "H5ZjpeglsDecodeNominal";
static const char* H5_ZJPEGLS_ENCODE_CANNOT_APPLY = "H5ZjpeglsEncodeCannotApply";
static const char* H5_ZJPEGLS_ENCODE_INEFFECTIVE = "H5ZjpeglsEncodeIneffective";
static const char* H5_ZJPEGLS_ENCODE_RGBA_NOMINAL = "H5ZjpeglsEncodeRGBANominal";
static const char* H5_ZJPEGLS_ENCODE_NOMINAL = "H5ZjpeglsEncodeNominal";

// Declare the tests
// Encode nominal
int H5ZjpeglsEncodeNominal(int argc, char* argv[]);
// Encode RGBA
int H5ZjpeglsEncodeRGBANominal(int argc, char* argv[]);
// Encode cannot be applied
int H5ZjpeglsEncodeCannotApply(int argc, char* argv[]);
// Decode nominal
int H5ZjpeglsDecodeNominal(int argc, char* argv[]);

// Main test function
int main(int argc, char* argv[])
{
	int testResult = EXIT_FAILURE;

    // Get the test name
    char * testName = argv[FCI_ONE];

    // Call the test functions
	if (strcmp(testName, H5_ZJPEGLS_ENCODE_NOMINAL) == 0) {
        // launch the test
    	testResult = H5ZjpeglsEncodeNominal(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_ENCODE_RGBA_NOMINAL) == 0) {
        // launch the test
    	testResult = H5ZjpeglsEncodeRGBANominal(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_ENCODE_INEFFECTIVE) == 0) {
        // launch the test
    	// use the same encoding program as for the H5ZjpeglsEncodeNominal test
    	testResult = H5ZjpeglsEncodeNominal(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_ENCODE_CANNOT_APPLY) == 0) {
        // launch the test
    	testResult = H5ZjpeglsEncodeCannotApply(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_DECODE_NOMINAL) == 0) {
        // launch the test
    	testResult = H5ZjpeglsDecodeNominal(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_DECODE_RGBA_NOMINAL) == 0) {
        // launch the test
    	// use the same decoding program for the RGBA image as for the gray scale image
    	testResult = H5ZjpeglsDecodeNominal(argc-1, argv+1);
    }
	if (strcmp(testName, H5_ZJPEGLS_DECODE_INEFFECTIVE) == 0) {
        // launch the test
    	// use the same decoding program for the noise image as for the gray scale image
    	testResult = H5ZjpeglsDecodeNominal(argc-1, argv+1);
    }

    return testResult;
}
