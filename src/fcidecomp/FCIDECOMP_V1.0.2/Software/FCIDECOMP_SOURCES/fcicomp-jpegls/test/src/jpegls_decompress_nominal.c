// $Id: jpegls_decompress_nominal.c 778 2016-03-09 07:56:29Z delaunay $
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




// Nominal decompression of a test image
int jpeglsDecompressNominal(int argc, char* argv[]) {
	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != FCI_THREE) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS)
	}

	// Get the input arguments
	// Input FileName
	char * inFile = argv[FCI_ONE];
	// Output FileName
	char * outFile = argv[FCI_TWO];

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

	// Number of samples in the input image
	size_t nSamples = samples * lines;
	// Number of bytes
	int bpp = jlsParams.bit_per_sample;
	int nBytes = (bpp > 0) ? (((bpp - 1) / 8) + 1) : 0;
	// Size of the output buffer
	size_t outSize = nSamples * nBytes;

	// Allocate memory for the decompressed image
	short *outBuf = (short *) malloc(outSize);
	if (outBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Decompress
	if (jpeglsDecompress(outBuf, outSize, inBuf, inSize) != FJLS_NOERR) {
		ERR_TEST(EXIT_FAILURE, "Error during the decompression!\n");
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
	size_t nBytesWritten = fwrite(outBuf, 1, outSize, pFile);
	if (nBytesWritten != outSize) {
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
