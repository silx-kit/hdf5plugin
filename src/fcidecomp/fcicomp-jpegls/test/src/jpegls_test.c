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

static const char* JPEG_COMPRESS_NOMINAL_TEST 		= "jpeglsCompressNominal";
static const char* JPEG_COMPRESS_NOISE_TEST 		= "jpeglsCompressNoise";
static const char* JPEG_DECOMPRESS_NOISE_TEST 		= "jpeglsDecompressNoise";
static const char* JPEG_DECOMPRESS_NOMINAL_TEST 	= "jpeglsDecompressNominal";
// static const char* JPEG_COMPRESS_ERROR_CASE			= "jpeglsCompressErrorCase";
static const char* JPEG_READ_HEADER_ERROR_CASE		= "jpeglsReadHeaderErrorCase";
static const char* JPEG_DECOMPRESS_ERROR_CASE		= "jpeglsDecompressErrorCase";

// Declare the tests
// Compress nominal
int jpeglsCompressNominal(int argc, char* argv[]);
// Decompress nominal
int jpeglsDecompressNominal(int argc, char* argv[]);
// Compress error case
// int jpeglsCompressErrorCase(int argc, char* argv[]);
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
	// if (strcmp(testName, JPEG_COMPRESS_ERROR_CASE) == 0) {
        // launch the test
		// result = jpeglsCompressErrorCase(argc-1, argv+1);
    // }
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
