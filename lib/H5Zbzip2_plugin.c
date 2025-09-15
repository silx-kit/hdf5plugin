/*
 * Dynamically loaded filter plugin for HDF5 BZip2 filter.
 *
 *
 */

#include "H5Zbzip2.h"
#include "H5PLextern.h"

size_t bzip2_deflate(unsigned int flags, size_t cd_nelmts,
                     const unsigned int cd_values[], size_t nbytes,
                     size_t *buf_size, void **buf);


const H5Z_class_t H5Z_BZIP2[1] = {
  {
    H5Z_CLASS_T_VERS,             /* H5Z_class_t version */
    (H5Z_filter_t)(FILTER_BZIP2), /* filter_id */
    1, 1,                         /* Encoding and decoding enabled */
    "bzip2",                      /* comment */
    NULL,                         /* can_apply_func */
    NULL,                         /* set_local_func */
    (H5Z_func_t)(bzip2_deflate)   /* filter_func */
  }
};

H5PL_type_t H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
const void *H5PLget_plugin_info(void) {return H5Z_BZIP2;}
