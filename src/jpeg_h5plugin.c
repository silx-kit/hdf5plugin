/*
 * Dynamically loaded filter plugin for HDF5 JPEG filter.
 *
 * Author: Mark Rivers <rivers@cars.uchicago.edu>
 * Created: 2019
 *
 */


#include "jpeg_h5filter.h"
#include "H5PLextern.h"

H5PL_type_t H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
const void* H5PLget_plugin_info(void) {return jpeg_H5Filter;}

