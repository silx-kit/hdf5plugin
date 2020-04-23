// $Id: H5Zjpegls_encode_cannot_apply.c 778 2016-03-09 07:56:29Z delaunay $
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
#include "fcicomp_jpegls.h"
#include "H5Zjpegls.h"
#include "fcicomp_errors.h"
#include "fcicomp_options.h"


// Define the names of the datasets
#define UNSUPPORTED_DTYPE_DATASET				"Unsupported data type dataset"
#define UNSUPPORTED_DBYTE_DATASET				"Unsupported data bytes dataset"
#define INVALID_NUMBER_OF_COMPONENTS_DATASET	"Invalid number of components dataset"
#define INVALID_NUMBER_OF_DIMENSIONS_DATASET	"Invalid number of dimensions dataset"
#define TOO_SMALL_DATASET						"Too small dataset"


// Test the case where the can_apply function returns false
int H5ZjpeglsEncodeCannotApply(int argc, char* argv[]) {
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

	// -----------------------------------------------
	// Create the HDF5 file
	// -----------------------------------------------

	// Create a new HDF5 file using the default properties
	hid_t file = H5Fcreate(outFile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	// ----------------------------------------------------
	// Create a dataset but with a data type not supported
	// ----------------------------------------------------

	// Set the HDF5 dataset dimensions
	hsize_t dims[FCI_THREE] = { 1, lines, samples };

	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space1 = H5Screate_simple(FCI_THREE, dims, NULL);

	// Create the dataset creation property list
	hid_t dcpl1 = H5Pcreate(H5P_DATASET_CREATE);

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl1, 0);

	// Add the JPEG-LS compression filter
	H5Pset_filter(dcpl1, H5Z_FILTER_JPEGLS, H5Z_FLAG_OPTIONAL, 0, NULL);

	// Check that filter is registered with the library
	if (H5Zfilter_avail(H5Z_FILTER_JPEGLS)) {
		// If it is registered, retrieve filter's configuration
		unsigned int filter_config = 0;
		H5Zget_filter_info(H5Z_FILTER_JPEGLS, &filter_config);
		// Check that the encoder is available
		if ((filter_config & H5Z_FILTER_CONFIG_ENCODE_ENABLED) != 1) {
			ERR_TEST(EXIT_FAILURE, JPEG_LS_FILTER_UNVAILABLE);
		}
	} else {
		// If the filter is not registered, print an error
		ERR_TEST(EXIT_FAILURE, JPEG_LS_FILTER_UNVAILABLE);
	}

	// Define the chunked layout since filters can only be used with chunked layout
	hsize_t chunk[FCI_THREE] = { 1, lines, samples };
	H5Pset_chunk(dcpl1, FCI_THREE, chunk);

	// Set a wrong data type for JPEG-LS filter
	hid_t type_id = H5T_IEEE_F32LE;

	// Create the dataset using the dataset creation property list we have created
	hid_t dset1 = H5Dcreate(file, UNSUPPORTED_DTYPE_DATASET, type_id, space1,
	H5P_DEFAULT, dcpl1, H5P_DEFAULT);

	// Write the data to the dataset we have created
	hid_t mem_type_id = H5T_NATIVE_SHORT;
	H5Dwrite(dset1, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, inBuf);

	// Close the dataset creation property list
	H5Pclose(dcpl1);
	// Close the dataspace
	H5Sclose(space1);
	// Close the dataset
	H5Dclose(dset1);



	// -----------------------------------------------------------
	// Create a dataset but with an invalid number of components
	// -----------------------------------------------------------

	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space2 = H5Screate_simple(FCI_THREE, dims, NULL);

	// Create the dataset creation property list
	hid_t dcpl2 = H5Pcreate(H5P_DATASET_CREATE);

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl2, 0);

	// Add the JPEG-LS compression filter
	H5Pset_filter(dcpl2, H5Z_FILTER_JPEGLS,	H5Z_FLAG_OPTIONAL, 0, NULL);

	// Define the chunked layout since filters can only be used with chunked layout
	H5Pset_chunk(dcpl2, FCI_THREE, chunk);

	// Set a wrong data type for JPEG-LS filter
	type_id = H5T_STD_I32LE;

	// Create the dataset using the dataset creation property list we have created
	hid_t dset2 = H5Dcreate(file, UNSUPPORTED_DBYTE_DATASET, type_id, space2,
	H5P_DEFAULT, dcpl2, H5P_DEFAULT);

	// Write the data to the dataset we have created
	mem_type_id = H5T_NATIVE_SHORT;
	H5Dwrite(dset2, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, inBuf);

	// Close the dataset creation property list
	H5Pclose(dcpl2);
	// Close the dataspace
	H5Sclose(space2);
	// Close the dataset
	H5Dclose(dset2);

	// ------------------------------------------------------
	// Create a dataset but with a wrong number of components
	// ------------------------------------------------------

	// Set the HDF5 dataset dimensions
	hsize_t invalidDims[FCI_THREE] = {FCI_FIVE, lines/FCI_FIVE, samples };

	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space3 = H5Screate_simple(FCI_THREE, invalidDims, NULL);

	// Create the dataset creation property list
	hid_t dcpl3 = H5Pcreate(H5P_DATASET_CREATE);

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl3, 0);

	// Add the JPEG-LS compression filter
	H5Pset_filter(dcpl3, H5Z_FILTER_JPEGLS,	H5Z_FLAG_OPTIONAL, 0, NULL);

	// Define the chunked layout since filters can only be used with chunked layout
	H5Pset_chunk(dcpl3, FCI_THREE, invalidDims);

	// Create the dataset using the dataset creation property list we have created
	type_id = H5T_STD_I16LE;
	hid_t dset3 = H5Dcreate(file, INVALID_NUMBER_OF_COMPONENTS_DATASET, type_id, space3,
	H5P_DEFAULT, dcpl3, H5P_DEFAULT);

	// Write the data to the dataset we have created
	mem_type_id = H5T_NATIVE_SHORT;
	H5Dwrite(dset3, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, inBuf);

	// Close the dataset creation property list
	H5Pclose(dcpl3);
	// Close the dataspace
	H5Sclose(space3);
	// Close the dataset
	H5Dclose(dset3);


	// ----------------------------------------------------
	// Create a dataset but with too many dimensions
	// ----------------------------------------------------

	// Set the HDF5 dataset dimensions
	hsize_t invalidRankDims[FCI_FOUR] = { 1, 1, lines, samples };

	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space4 = H5Screate_simple(FCI_FOUR, invalidRankDims, NULL);

	// Create the dataset creation property list
	hid_t dcpl4 = H5Pcreate(H5P_DATASET_CREATE);

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl4, 0);

	// Add the JPEG-LS compression filter
	H5Pset_filter(dcpl4, H5Z_FILTER_JPEGLS,	H5Z_FLAG_OPTIONAL, 0, NULL);

	// Define the chunked layout since filters can only be used with chunked layout
	H5Pset_chunk(dcpl4, FCI_FOUR, invalidRankDims);

	// Create the dataset using the dataset creation property list we have created
	type_id = H5T_STD_I16LE;
	hid_t dset4 = H5Dcreate(file, INVALID_NUMBER_OF_DIMENSIONS_DATASET, type_id, space4,
	H5P_DEFAULT, dcpl4, H5P_DEFAULT);

	// Write the data to the dataset we have created
	mem_type_id = H5T_NATIVE_SHORT;
	H5Dwrite(dset4, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, inBuf);

	// Close the dataset creation property list
	H5Pclose(dcpl4);
	// Close the dataspace
	H5Sclose(space4);
	// Close the dataset
	H5Dclose(dset4);

	// -----------------------------------------------------------
	// Create a dataset too small
	// -----------------------------------------------------------

	// Set the HDF5 dataset dimensions
	hsize_t smallDims[FCI_TWO] = { FCI_TWO, FCI_TWO };

	// Create a dataspace
	// Setting maximum size to NULL sets the maximum size to be the current size
	hid_t space5 = H5Screate_simple(FCI_TWO, smallDims, NULL);

	// Create the dataset creation property list
	hid_t dcpl5 = H5Pcreate(H5P_DATASET_CREATE);

	// Turn off the create/modify/access time tracking for objects created
	// so that everything is bit-for-bit reproducible
	H5Pset_obj_track_times(dcpl5, 0);

	// Add the JPEG-LS compression filter
	H5Pset_filter(dcpl5, H5Z_FILTER_JPEGLS,	H5Z_FLAG_OPTIONAL, 0, NULL);

	// Define the chunked layout since filters can only be used with chunked layout
	H5Pset_chunk(dcpl5, FCI_TWO, smallDims);

	// Create the dataset using the dataset creation property list we have created
	type_id = H5T_STD_I16LE;
	hid_t dset5 = H5Dcreate(file, TOO_SMALL_DATASET, type_id, space5,
	H5P_DEFAULT, dcpl5, H5P_DEFAULT);

	// Write the data to the dataset we have created
	mem_type_id = H5T_NATIVE_SHORT;
	H5Dwrite(dset5, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, inBuf);

	// Close the dataset creation property list
	H5Pclose(dcpl5);
	// Close the dataspace
	H5Sclose(space5);
	// Close the dataset
	H5Dclose(dset5);

	// ------------------------------------------
	// Cleanup
	// ------------------------------------------

	// Close the HDF5 file
	H5Fclose(file);
	// Release resources
	H5close();

	// Free memory
	free(inBuf);

	return EXIT_SUCCESS;
}
