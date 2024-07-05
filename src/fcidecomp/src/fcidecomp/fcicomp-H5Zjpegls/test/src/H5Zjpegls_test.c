// =============================================================
//
// Copyright 2015-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// =============================================================

// AUTHORS:
// - THALES Services

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
