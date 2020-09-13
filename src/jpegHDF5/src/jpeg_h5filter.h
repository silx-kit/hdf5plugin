/*
 * JPEG HDF5 filter
 *
 * Author: Mark Rivers <rivers@cars.uchicago.edu>
 * Created: 2019
 *
 */


#ifndef JPEG_H5FILTER_H
#define JPEG_H5FILTER_H

#define H5Z_class_t_vers 2
#include "hdf5.h"

#define JPEG_H5FILTER 32019


H5_DLLVAR H5Z_class_t jpeg_H5Filter[1];


/* ---- jpeg_register_h5filter ----
 *
 * Register the JPEG HDF5 filter within the HDF5 library.
 *
 * Call this before using the JPEG HDF5 filter from C unless
 * using dynamically loaded filters.
 *
 */
int jpeg_register_h5filter(void);


#endif // JPEG_H5FILTER_H
