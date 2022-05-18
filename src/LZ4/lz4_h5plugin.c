/*
 * Dynamically loaded filter plugin for HDF5 LZ4 filter.
 *
 *
 */


#include "lz4_h5filter.h"
#include "H5PLextern.h"

H5PL_type_t H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
const void *H5PLget_plugin_info(void) {return H5Z_LZ4;}
