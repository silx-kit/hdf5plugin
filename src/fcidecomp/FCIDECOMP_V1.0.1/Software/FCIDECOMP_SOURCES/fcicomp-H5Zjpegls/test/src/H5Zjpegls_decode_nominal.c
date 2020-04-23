// $Id: H5Zjpegls_decode_nominal.c 778 2016-03-09 07:56:29Z delaunay $
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

#include "hdf5.h"
#include "H5Zjpegls.h"

#include "fcicomp_errors.h"
#include "fcicomp_options.h"


#define DATASET_NAME	"image"

// Nominal compression of a test image
int H5ZjpeglsDecodeNominal(int argc, char* argv[]) {

	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != FCI_THREE) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS);
	}

	// Get the input arguments
	// Input FileName
	char * inFile = argv[FCI_ONE];
	// Output FileName
	char * outFile = argv[FCI_TWO];

	// -----------------------------------------------
	// Read the HDF5 file compressed with JPEG-LS
	// -----------------------------------------------

	// Open file and dataset using the default properties
	hid_t file = H5Fopen(inFile, H5F_ACC_RDONLY, H5P_DEFAULT);
	hid_t dset = H5Dopen(file, DATASET_NAME, H5P_DEFAULT);

	// Retrieve dataset creation property list
	hid_t dcpl = H5Dget_create_plist(dset);

	// Retrieve the filter id and the JPEG-LS compression parameters
	unsigned int flags = 0;
	size_t cd_nelmts = H5Z_FILTER_JPEGLS_NPARAMS;
	unsigned int cd_values[H5Z_FILTER_JPEGLS_NPARAMS] = { 0 };
	unsigned int filter_config = 0;
	H5Z_filter_t filter_id = H5Pget_filter2(dcpl, 0, &flags,
			&cd_nelmts, cd_values, 0, NULL, &filter_config);
	if (filter_id != H5Z_FILTER_JPEGLS) {
		ERR_TEST(EXIT_FAILURE, UNEXPECTED_FILTER);
	}

	// Get the filter parameters
	jls_filter_parameters_t * params = (jls_filter_parameters_t *) cd_values;

	// Get the dataset dimensions
	unsigned int components = params->dims[0];
	unsigned int lines = params->dims[FCI_ONE];
	unsigned int samples = params->dims[FCI_TWO];

	// Get the number of bytes per samples in the uncompressed dataset
	unsigned int dataBytes = params->dataBytes;

	// Set the default memory type for the dataset
	hid_t mem_type_id = H5T_NATIVE_UCHAR;
	// Change the memory type for short data type
	if (dataBytes == FCI_TWO) {
		mem_type_id = H5T_NATIVE_SHORT;
	}

	// Compute the size of the output buffer
	size_t outBufSize = components * lines * samples * dataBytes;

	// Allocate memory for the dataset
	void * outBuf = malloc(outBufSize);
	if (outBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Read the data using the default properties
	herr_t status = H5Dread(dset, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT,
			outBuf);

	// Close and release resources
	status = H5Pclose(dcpl);
	status = H5Dclose(dset);
	status = H5Fclose(file);

	// ------------------------------------------
	// Write the output buffer to file
	// ------------------------------------------

	// Open the file in the write mode
	FILE * pFile = fopen(outFile, FCI_WRITE);
	if (pFile == NULL) {
		ERR_TEST(EXIT_FAILURE, CANNOT_OPEN_FILE_W);
	}

	// Write the data into the file
	size_t result = fwrite(outBuf, 1, outBufSize, pFile);
	if (result != outBufSize) {
		ERR_TEST(EXIT_FAILURE, ERROR_WRITING_FILE);
	}

	// Close the file
	fclose(pFile);

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Free memory
	free(outBuf);

	return EXIT_SUCCESS;
}
