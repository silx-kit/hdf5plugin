/*
 * This file contains the clamp filter class definition, H5Z_clamp_class_t,
 * and necessary functions required by the HDF5 plugin architecture:
 * - can_apply()
 * - set_local()
 * - filter()
 * - get_plugin_info()
 * - get_plugin_type()
 */

#define H5Z_FILTER_CLAMP 45678

#include <assert.h>
#include <stdlib.h>

#include <H5PLextern.h>
#include <hdf5.h>

static htri_t H5Z_can_apply_clamp(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
  /*
   * 	dcpl_id	  Dataset creation property list identifier
   * 	type_id	  Datatype identifier
   * 	space_id  Dataspace identifier
   */

  /* Get datatype class. Fail if not floats. */
  if (H5Tget_class(type_id) != H5T_FLOAT) {
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
            "bad data type. Only floats are supported in H5Z-CLAMP");
    return 0;
  }
  size_t type_size = H5Tget_size(type_id);
  assert(type_size == 4 || type_size == 8);

  return 1;
}

static herr_t H5Z_set_local_clamp(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
  /*
   * 	dcpl_id	  Dataset creation property list identifier
   * 	type_id	  Datatype identifier
   * 	space_id  Dataspace identifier
   */

  /* Get the datatype size. It must be 4 or 8, since the float type is verified by `can_apply`. */
  unsigned int is_float = H5Tget_size(type_id) == 4 ? 1 : 0;

  /*
   * Assemble the meta info to be stored.
   * [0] : float/double
   */
  size_t cd_nelems = 1;
  unsigned int cd_values[1] = {is_float};
  H5Pmodify_filter(dcpl_id, H5Z_FILTER_CLAMP, H5Z_FLAG_MANDATORY, cd_nelems, cd_values);

  return 1;
}

static size_t H5Z_filter_clamp(unsigned int flags,
                               size_t cd_nelmts,
                               const unsigned int cd_values[],
                               size_t nbytes,
                               size_t* buf_size,
                               void** buf)
{
  int is_float = cd_values[0];
  assert(is_float == 1 || is_float == 0);

  if (flags & H5Z_FLAG_REVERSE) { /* Decompression */

    if (is_float) {
      if (nbytes % 4) {
        H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADSIZE,
                "Decompression: input buffer len isn't right.");
        return 0;
      }
      const size_t nelem = nbytes / 4;
      float* p = (float*)(*buf);
      for (size_t i = 0; i < nelem; i++) {
        if (p[i] < 0.f)
          p[i] = 0.f;
      }
    }
    else {
      if (nbytes % 8) {
        H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADSIZE,
                "Decompression: input buffer len isn't right.");
        return 0;
      }
      const size_t nelem = nbytes / 8;
      double* p = (double*)(*buf);
      for (size_t i = 0; i < nelem; i++) {
        if (p[i] < 0.0)
          p[i] = 0.0;
      }
    }

    return *buf_size;

  }      /* Finish Decompression */
  else { /* Compression */

    /* Nothing to be done during compression :) */
    return *buf_size;
  }
}

const H5Z_class2_t H5Z_clamp_class_t = {H5Z_CLASS_T_VERS, /* H5Z_class_t version */
                                        H5Z_FILTER_CLAMP, /* Filter id number    */
                                        1,                /* encoder_present flag (set to true) */
                                        1,                /* decoder_present flag (set to true) */
                                        "H5Z-CLAMP",      /* Filter name for debugging  */
                                        H5Z_can_apply_clamp, /* The "can apply" callback   */
                                        H5Z_set_local_clamp, /* The "set local" callback   */
                                        H5Z_filter_clamp};   /* The actual filter function */

const void* H5PLget_plugin_info()
{
  return &H5Z_clamp_class_t;
}

H5PL_type_t H5PLget_plugin_type()
{
  return H5PL_TYPE_FILTER;
}
