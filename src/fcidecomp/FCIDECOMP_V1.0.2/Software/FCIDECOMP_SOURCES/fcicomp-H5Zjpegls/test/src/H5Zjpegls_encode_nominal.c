// $Id: H5Zjpegls_encode_nominal.c 778 2016-03-09 07:56:29Z delaunay $
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


#define FALSE			0
#define TRUE			1
#define DATASET_NAME	"image"

// Error message
#define ERROR_JLS_NOT_AVAILABLE_MSG	"JPEG-LS filter not available!\n"

// Nominal compression of a test image
int H5ZjpeglsEncodeNominal(int argc, char* argv[]) {
	// ------------------------------------------
	// Parse the input arguments
	// ------------------------------------------

	// Check the number of input arguments
	if (argc != FCI_SIX) {
		ERR_TEST(EXIT_FAILURE, INVALID_NUMBER_ARGUMENTS);
	}

	// Get the input arguments
	// Input FileName
	char * inFile = argv[FCI_ONE];
	// Output FileName
	char * outFile = argv[FCI_TWO];
	// Parameters
	int samples = atoi(argv[FCI_THREE]);
	int lines = atoi(argv[FCI_FOUR]);
	int bpp = atoi(argv[FCI_FIVE]);

	// ------------------------------------------
	// Read the input RAW file
	// ------------------------------------------

	// Compute the size of one sample in bytes
	int nBytes = (bpp > 0) ? (((bpp - 1) / FCI_EIGHT) + 1) : 0;
	// Size of the input buffer
	size_t inSize = samples * lines * nBytes;

	// Allocate memory
	char *inBuf = (char *) malloc(inSize);
	if (inBuf == NULL) {
		ERR_TEST(EXIT_FAILURE, MEMORY_ALLOCATION_ERROR);
	}

	// Open the file
	FILE * pFile = fopen(inFile, FCI_READ);
	if (pFile == NULL) {
		ERR_TEST(EXIT_FAILURE, CANNOT_OPEN_FILE_R);
	}
	// Read the file
	size_t result = fread(inBuf, 1, inSize, pFile);
	if (result != inSize) {
		ERR_TEST(EXIT_FAILURE, ERROR_READING_FILE);
	}
	// Close the file
	fclose(pFile);

	// ------------------------------------------
	// Set the JPEG-LS parameters
	// ------------------------------------------

	// Define the JPEG-LS coding parameters
	jls_parameters_t jlsParams = { 0, 0, 0, 0,{ 0, 0, 0, 0, 0}};

	// -----------------------------------------------
	// Create the HDF5 file compressed with JPEG-LS
	// -----------------------------------------------

	// Set the HDF5 dataset dimensions
	hsize_t dims[FCI_TWO] = { lines, samples };

	// Create a new HDF5 file using the default properties
	hid_t file = H5Fcreate(outFile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);


	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space = H5Screate_simple(FCI_TWO, dims, NULL);

	// Create the dataset creation property list
	hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);


	// Add the JPEG-LS compression filter and provide the JPEG-LS parameters
	herr_t status = H5Pset_filter(dcpl, H5Z_FILTER_JPEGLS,
			H5Z_FLAG_OPTIONAL, H5Z_FILTER_JPEGLS_USER_NPARAMS,
			(const unsigned int *) &jlsParams);

//	// Also works with default JPEG-LS parameters if 0, NULL is provided instead
//	herr_t status = H5Pset_filter(dcpl, H5Z_FILTER_JPEGLS,
//					H5Z_FLAG_OPTIONAL, 0, NULL);

	// Check that filter is registered with the library
	if (H5Zfilter_avail(H5Z_FILTER_JPEGLS)) {
		// If it is registered, retrieve filter's configuration
		unsigned int filter_config = 0;
		status = H5Zget_filter_info(H5Z_FILTER_JPEGLS, &filter_config);
		// Check that the encoder is available
		if ((filter_config & H5Z_FILTER_CONFIG_ENCODE_ENABLED) != 1) {
			ERR_TEST(EXIT_FAILURE, JPEG_LS_FILTER_UNVAILABLE);
		}
	} else {
		// If the filter is not registered, print an error
		ERR_TEST(EXIT_FAILURE, ERROR_JLS_NOT_AVAILABLE_MSG);
	}

	// Define the chunked layout since filters can only be used with chunked layout
	hsize_t chunk[FCI_TWO] = { lines, samples };
	status = H5Pset_chunk(dcpl, FCI_TWO, chunk);

	// Set the default memory type for the dataset
	hid_t type_id = H5T_STD_U8LE;
	hid_t mem_type_id = H5T_NATIVE_UCHAR;
	// Change the memory type for short data type
	if (nBytes == FCI_TWO) {
		type_id = H5T_STD_I16LE;
		mem_type_id = H5T_NATIVE_SHORT;
	}

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl, FALSE);

	// Create the dataset using the dataset creation property list we have created
	hid_t dset = H5Dcreate(file, DATASET_NAME, type_id, space, H5P_DEFAULT,
			dcpl, H5P_DEFAULT);


	// Write the data to the dataset we have created
	status = H5Dwrite(dset, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT,
			inBuf);

	// Close the dataset creation property list
	status = H5Pclose(dcpl);
	// Close the dataspace
	status = H5Sclose(space);
	// Close the dataset
	status = H5Dclose(dset);
	// Close the HDF5 file
	status = H5Fclose(file);
	// Release resources
	status = H5close();

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Free memory
	free(inBuf);

	return EXIT_SUCCESS;
}
