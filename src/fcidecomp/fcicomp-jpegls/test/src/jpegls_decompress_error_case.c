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




// Error case for the decompression of a test image
int jpeglsDecompressErrorCase(int argc, char* argv[]) {
	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != FCI_TWO) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS);
	}

	// Input FileName
	char * inFile = argv[FCI_ONE];

	// ------------------------------------------
	// Read the input JLS file
	// ------------------------------------------

	// Open the file
	FILE * pFile = fopen(inFile, "rb");
	if (pFile == NULL) {
		ERR_TEST(EXIT_FAILURE, CANNOT_OPEN_FILE_R);
	}

	// Obtain file size
	fseek(pFile, 0, SEEK_END);
	size_t inSize = ftell(pFile);
	// Replace the pointer at the beginning of the file
	rewind(pFile);

	// Allocate memory to read the compressed file
	char *inBuf = (char *) malloc(inSize);
	if (inBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Read the compressed file
	size_t nBytesRead = fread(inBuf, 1, inSize, pFile);
	if (nBytesRead != inSize) {
		ERR_TEST(EXIT_FAILURE, ERROR_READING_FILE);
	}

	// Close the file
	fclose(pFile);

	// ------------------------------------------
	// Read the input JLS header
	// ------------------------------------------

	// Define the JPEG-LS coding parameters
	jls_parameters_t jlsParams = { 0, 0, 0, 0,{ 0, 0, 0, 0, 0}};

	// Read JPEG-LS header to get the image parameters
	int samples = 0;
	int lines = 0;
	if (jpeglsReadHeader(inBuf, inSize, &samples, &lines, &jlsParams) != FJLS_NOERR) {
		ERR_TEST(EXIT_FAILURE, "Error reading the JPEG-LS header!\n");
	}

	// ------------------------------------------
	// Perform JPEG-LS decompression
	// ------------------------------------------

	// Provoke an error: do not allocate enough memory for the output decompressed image
	size_t outSize = 1;
	// Allocate memory
	char *outBuf = (char *) malloc(outSize);
	if (outBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Decompress
	if (jpeglsDecompress(outBuf, outSize, inBuf, inSize) != FJLS_UNCOMPRESSED_BUFER_TOO_SMALL) {
		// As we provoke an error, jpeglsDecompress should return a positive error code
		ERR_TEST(EXIT_FAILURE, "The expected error has not occurred!\n");
	}

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Free memory
	free(inBuf);
	free(outBuf);

	// The expected error has happened => exit with the success status
	return EXIT_SUCCESS;
}
