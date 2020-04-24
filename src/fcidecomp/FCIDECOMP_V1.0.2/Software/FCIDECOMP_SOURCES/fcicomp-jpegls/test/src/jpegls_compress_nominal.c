// $Id: jpegls_compress_nominal.c 778 2016-03-09 07:56:29Z delaunay $
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

#include "fcicomp_jpegls.h"
#include "fcicomp_errors.h"
#include "fcicomp_options.h"


// Margin factor on the compressed buffer size. This allows allocating
// slightly more memory than necessary for the cases where the compression is
// not efficient e.g. noise image. Otherwise a buffer overflow may occur in
// CharLS and also a potential seg fault.
#define COMPRESSED_BUFFER_SIZE_MARGIN_FACTOR	1.2


// Nominal compression of a test image
int jpeglsCompressNominal(int argc, char* argv[]) {
	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != 6) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS)
	}

	// Get the input arguments
	// Input FileName
	char * inFile = argv[FCI_ONE];
	// Output FileName
	char * outFile = argv[FCI_TWO];
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
	void *inBuf = malloc(inSize);
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

	// Set the JPEG-LS coding parameters
	jlsParams.bit_per_sample = 8*nBytes;
	jlsParams.components = 1;

	// ------------------------------------------
	// Perform JPEG-LS compression
	// ------------------------------------------

	// Size of the output buffer
	// Allocate slightly more memory than necessary for the case the
	// compression is not efficient e.g. noise image. Otherwise this could
	// provoke a buffer overflow and a potential seg fault.
	size_t outSize = COMPRESSED_BUFFER_SIZE_MARGIN_FACTOR * nSamples * nBytes;

	// Allocate memory
	void *outBuf = malloc(outSize);
	if (outBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Compress
	size_t compressedSize = 0;
	int result = jpeglsCompress(outBuf, outSize, &compressedSize, inBuf, inSize,
			samples, lines, jlsParams);
	if (result != FJLS_NOERR) {
		ERR_TEST(EXIT_FAILURE, "Error during the compression!\n");
	}

	// ------------------------------------------
	// Write the output file
	// ------------------------------------------

	// Open the file in the write mode
	pFile = fopen(outFile, FCI_WRITE);
	if (pFile == NULL) {
		ERR_TEST(EXIT_FAILURE, CANNOT_OPEN_FILE_W);
	}

	// Write the data into the file
	size_t nBytesWritten = fwrite(outBuf, 1, compressedSize, pFile);
	if (nBytesWritten != compressedSize) {
		ERR_TEST(EXIT_FAILURE, ERROR_WRITING_FILE);
	}

	// Close the file
	fclose(pFile);

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Free memory
	free(inBuf);
	free(outBuf);

	return EXIT_SUCCESS;
}
