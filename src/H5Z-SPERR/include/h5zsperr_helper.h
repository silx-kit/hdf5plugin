/*
 * This file contains a few helper functions for the H5Z-SPERR filter.
 */

#ifndef H5ZSPERR_HELPER_H
#define H5ZSPERR_HELPER_H

#include <stdlib.h>

#define LARGE_MAGNITUDE_F 1e35f
#define LARGE_MAGNITUDE_D 1e35
#define H5ZSPERR_COMPATIBILITY 1

#ifdef __cplusplus
namespace C_API {
extern "C" {
#endif

/*
 * Pack and unpack additional information about the input data into an integer.
 * It returns the encoded unsigned int, which shouldn't be zero.
 * The packing function is called by `set_local()` to prepare information
 * for `H5Z_filter_sperr()`, which calls the unpack function to extract such info.
 */
unsigned int h5zsperr_pack_extra_info(int rank, int is_float, int missing_val_mode, int magic_num);
void h5zsperr_unpack_extra_info(unsigned int meta,
                                int* rank,
                                int* is_float,
                                int* missing_val_mode,
                                int* magic_num);

/*
 * Check if an input array really has missing values.
 */
int h5zsperr_has_nan(const void* buf, size_t nelem, int is_float);
int h5zsperr_has_large_mag(const void* buf, size_t nelem, int is_float);

/*
 * Produce a compact bitmask.
 * `mask_buf` is already allocated with length `mask_bytes`.
 * Returns 0 upon success.
 */
int h5zsperr_make_mask_nan(const void* data_buf, size_t nelem, int is_float,
                           void* mask_buf, size_t mask_bytes, size_t* useful_bytes);
int h5zsperr_make_mask_large_mag(const void* data_buf, size_t nelem, int is_float,
                                 void* mask_buf, size_t mask_bytes, size_t* useful_bytes);

/*
 * Replace every missing value in the `data_buf` with the mean of the field.
 */
float h5zsperr_treat_nan_f32(float* data_buf, size_t nelem);
double h5zsperr_treat_nan_f64(double* data_buf, size_t nelem);
float h5zsperr_treat_large_mag_f32(float* data_buf, size_t nelem);
double h5zsperr_treat_large_mag_f64(double* data_buf, size_t nelem);

#ifdef __cplusplus
} /* end of extern "C" */
} /* end of namespace C_API */
#endif

#endif
