/*
 * LZ4 HDF5 filter
 *
 * Header File
 *
 * Filter Options
 * --------------
 *  block_size (option slot 0) : interger (optional)
 *      What block size to use. Default is 0,
 *      for which lz4 will pick a block size.
 *  number_of_threads (option slot 1) : Not currently implemented
 *
 *      The compressed format of the data is described in
 *      http://www.hdfgroup.org/services/filters/HDF5_LZ4.pdf.
 *
 */


#ifndef LZ4_H5FILTER_H
#define LZ4_H5FILTER_H

#define H5Z_class_t_vers 2
#include "hdf5.h"

#define H5Z_FILTER_LZ4 32004


H5_DLLVAR const H5Z_class2_t H5Z_LZ4[1];

/* ---- lz4_register_h5filter ----
 *
 * Register the LZ4 HDF5 filter within the HDF5 library.
 *
 * Call this before using the bitshuffle HDF5 filter from C unless
 * using dynamically loaded filters.
 *
 */
int lz4_register_h5filter(void);


#endif // LZ4_H5FILTER_H
