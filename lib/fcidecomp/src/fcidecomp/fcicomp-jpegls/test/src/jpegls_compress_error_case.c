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

#include "fcicomp_jpegls.h"
#include "fcicomp_errors.h"
#include "fcicomp_options.h"




// Error case for the compression of a test image
int jpeglsCompressErrorCase(int argc, char* argv[]) {
	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != 6) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS);
	}

	// Get the input arguments
	// Input FileName
	char * inFile = argv[FCI_ONE];
	// Output FileName
	// char * outFile = argv[FCI_TWO];
	// Parameters
	int samples = atoi(argv[FCI_THREE]);
	int lines = atoi(argv[4]);
	int bpp = atoi(argv[5]);

	// ------------------------------------------
	// Read the input RAW file
	// ------------------------------------------

	// Number of samples in the input image
	size_t nSamples = samples * lines;
	// Compute the size of one sample in bytes
	int nBytes = (bpp > 0) ? (((bpp - 1) / 8) + 1) : 0;
	// Size of the input buffer
	size_t inSize = nSamples * nBytes;

	// Allocate memory
	char *inBuf = (char *) malloc(inSize);
	if (inBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Open the file
	FILE * pFile = fopen(inFile, "rb");
	if (pFile == NULL) {
		ERR_TEST(EXIT_FAILURE, CANNOT_OPEN_FILE_R);
	}
	// Read the file
	size_t nBytesRead = fread(inBuf, 1, inSize, pFile);
	if (nBytesRead != inSize) {
		ERR_TEST(EXIT_FAILURE, ERROR_READING_FILE);
	}
	// Close the file
	fclose(pFile);

	// ------------------------------------------
	// Set the JPEG-LS parameters
	// ------------------------------------------

	// Define the JPEG-LS coding parameters
	jls_parameters_t jlsParams = { 0, 0, 0, 0,{ 0, 0, 0, 0, 0}};

	// Allocate memory for the compressed buffer
	size_t outSize = inSize;
	// Allocate memory
	char *outBuf = (char *) malloc(outSize);
	if (outBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// ------------------------------------------------------------
	// Perform JPEG-LS compression with invalid JPEG-LS parameters
	// ------------------------------------------------------------

	// Set the JPEG-LS parameters
	// Provoke an error: set wrong JPEG-LS parameters
	jlsParams.bit_per_sample = 32;
	jlsParams.components = 1;

	// Compress
	size_t compressedSize = 0;
	int result = jpeglsCompress(outBuf, outSize, &compressedSize, inBuf, inSize,
			samples, lines, jlsParams);
	if ((result != FJLS_INVALID_JPEGLS_PARAMETERS) && (result != FJLS_UNSUPPORTED_JPEGLS_PARAMETERS)) {
		// As we provoke an error, jpeglsCompress should return a positive error code
		ERR_TEST(EXIT_FAILURE, "The expected error has not occurred!\n");
	}

//	// ------------------------------------------------------------
//	// Perform JPEG-LS compression with invalid JPEG-LS parameters
//	// ------------------------------------------------------------
//
//	// Set the JPEG-LS parameters
//	// Provoke an error: set wrong JPEG-LS parameters
//	jlsParams->bit_per_sample = 16;
//	jlsParams->components = 5;
//
//	// Compress
//	result = jpeglsCompress(outBuf, outSize, &compressedSize, inBuf, inSize,
//			samples, lines, jlsParams);
//	if (result != FJLS_INVALID_JPEGLS_PARAMETERS) {
//		// As we provoke an error, jpeglsCompress should return a positive error code
//		fputs("The expected error has not occurred!\n", stderr);
//		return EXIT_FAILURE;
//	}

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Free memory
	free(inBuf);
	free(outBuf);

	// We had the expected error => exit with the success status
	return EXIT_SUCCESS;
}
