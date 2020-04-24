// $Id: H5Zjpegls.h 824 2017-09-05 16:14:26Z delaunay $
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
// VERSION:1.0.2:NCR:FCICOMP-13:05/09/2017:Set the filter ID to 32018
// VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
//
// END-HISTORY
// =============================================================

#ifndef H5ZJPEGLS_H_
#define H5ZJPEGLS_H_

#include "fcicomp_jpegls.h"

/**
 * Define the filter ID.
 *
 * The filter identifier is designed to be a unique identifier for the filter.
 * Values from zero through 32,767 are reserved for filters supported by The
 * HDF Group in the HDF5 library and for filters requested and supported by
 * the 3rd party.
 *
 * Values from 32768 to 65535 are reserved for non-distributed uses (e.g.,
 * internal company usage) or for application usage when testing a feature.
 * The HDF Group does not track or document the usage of filters with
 * identifiers from this range.
 *
 * The filter ID has been provided by the HDF Group.
 */
#define H5Z_FILTER_JPEGLS			32018

/** Define the filter name */
#define H5Z_FILTER_JPEGLS_NAME		"JPEG-LS"

/** Maximum number of dimensions of the datasets compressed with the JPEG-LS filter.
 *
 * Allow color images.
 */
#define H5Z_FILTER_JPEGLS_MAX_NDIMS			3

/** Number of user parameters for controlling the filter.
 *
 * This is the number of cd_nelmts the user shall set at the call to
 * H5Pset_filter with the H5Z_FILTER_JPEGLS id.
 */
#define H5Z_FILTER_JPEGLS_USER_NPARAMS		(sizeof(jls_parameters_t)/sizeof(int))

/** Number of user parameters for controlling the filter.
 *
 * This is the number of cd_nelmts the user shall set at the call to
 * H5Pget_filter with the H5Z_FILTER_JPEGLS id.
 */
#define H5Z_FILTER_JPEGLS_NPARAMS			(1 + H5Z_FILTER_JPEGLS_MAX_NDIMS + H5Z_FILTER_JPEGLS_USER_NPARAMS)

/** Structure of the filter parameters.
 *
 * It contains only unsigned integers so that it may safely be casted
 * to an unsigned integer array cd_values[] at the input of the filter.
 */
typedef struct {
	unsigned int dataBytes; 						/**< data size (in bytes) */
	unsigned int dims[H5Z_FILTER_JPEGLS_MAX_NDIMS]; /**< dimension of the image: number of components, number of lines, number of columns. Fastest varying dimension last.*/
	jls_parameters_t jpeglsParameters; 				/**< user parameters for controlling the filter */
} jls_filter_parameters_t;


#endif /* H5ZJPEGLS_H_ */

