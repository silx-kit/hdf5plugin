#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <H5PLextern.h>
#include <hdf5.h>

#include <SPERR_C_API.h>
#include "h5z-sperr.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

static htri_t H5Z_can_apply_sperr(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
  /*
   * 	dcpl_id	  Dataset creation property list identifier
   * 	type_id	  Datatype identifier
   * 	space_id	Dataspace identifier
   */

  /* Get datatype class. Fail if not floats. */
  if (H5Tget_class(type_id) != H5T_FLOAT) {
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
            "bad data type. Only floats are supported in H5Z-SPERR");
    return 0;
  }

  /* Get the dataspace rank. Fail if not 2, 3, or 4. */
  int ndims = H5Sget_simple_extent_ndims(space_id);
  if (ndims < 2 || ndims > 4) {
#ifndef NDEBUG
    printf("%s: %d, ndims = %d\n", __FILE__, __LINE__, ndims);
#endif
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
            "bad dataspace ranks. Only rank==2, rank==3, or rank==4 with the time dimension==1 are "
            "supported in H5Z-SPERR");
    return 0;
  }

  /* Chunks have to be 2D, 3D, or 4D as well. */
  hsize_t chunks[4] = {0, 0, 0, 0};
  ndims = H5Pget_chunk(dcpl_id, 4, chunks);
  if (ndims < 2 || ndims > 4) {
#ifndef NDEBUG
    printf("%s: %d, ndims = %d\n", __FILE__, __LINE__, ndims);
#endif
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
            "bad chunk ranks. Only rank==2, rank==3, or rank==4 with the time dimension==1 are "
            "supported in H5Z-SPERR");
    return 0;
  }

  /* Find out the real dimension. */
  int real_dims = 0;
  for (int i = 0; i < 4; i++)
    if (chunks[i] > 1)
      real_dims++;
  if (real_dims < 2 || real_dims > 3) {
#ifndef NDEBUG
    printf("%s: %d, real_dims = %d\n", __FILE__, __LINE__, real_dims);
#endif
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
            "bad chunk dimensions: only true 2D slices or 3D volumes are supported in H5Z-SPERR");
    return 0;
  }

  /* Real chunk dimensions must be at least 9 */
  for (int i = 0; i < ndims; i++) {
    if (chunks[i] > 1 && chunks[i] < 9) {
#ifndef NDEBUG
      printf("%s: %d, chunks[%d] = %llu\n", __FILE__, __LINE__, i, chunks[i]);
#endif
      H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADTYPE,
              "bad chunk dimensions: any dimension must be at least 9. (may relax this requirement "
              "in the future)");
      return 0;
    }
  }

  return 1;
}

/*
 * Pack information about the input data into an `unsigned int`.
 * It returns the encoded unsigned int, which shouldn't be zero.
 */
static unsigned int H5Z_SPERR_pack_data_type(int rank,  /* Input */
                                             int dtype) /* Input */
{
  unsigned int ret = 0;

  /*
   * Bit position 0-3 to encode the rank.
   * Since this function is called from `set_local()`, it should always be 2 or 3.
   */
  if (rank == 2) {
    ret |= 1u << 1; /* Position 1 */
  }
  else {
    assert(rank == 3);
    ret |= 1u;      /* Position 0 */
    ret |= 1u << 1; /* Position 1 */
  }

  /*
   * Bit position 4-7 encode data type.
   * Only float (1) and double (0) are supported right now.
   */
  if (dtype == 1)   /* is_float   */
    ret |= 1u << 4; /* Position 4 */
  else
    assert(dtype == 0);

  return ret;
}

/*
 * Unpack information about the input data from an `unsigned int`.
 */
static void H5Z_SPERR_unpack_data_type(unsigned int meta, /* Input  */
                                       int* rank,         /* Output */
                                       int* dtype)        /* Output */
{
  /*
   * Extract rank from bit positions 0-3.
   */
  unsigned pos0 = meta & 1u;
  unsigned pos1 = meta & (1u << 1);
  if (!pos0 && pos1)
    *rank = 2;
  else if (pos0 && pos1)
    *rank = 3;
  else { /* error */
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADVALUE,
            "Rank is not 2 or 3.");
  }

  /*
   * Extract data type from position 4-7.
   * Only float and double are supported right now.
   */
  unsigned pos4 = meta & (1u << 4);
  if (pos4)
    *dtype = 1; /* is_float  */
  else
    *dtype = 0; /* is_double */
}

static herr_t H5Z_set_local_sperr(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
  /*
   * 	dcpl_id	  Dataset creation property list identifier
   * 	type_id	  Datatype identifier
   * 	space_id	Dataspace identifier
   */

  /* Get the user-specified compression mode and quality. */
  size_t user_cd_nelem = 2;
  unsigned int user_cd_values[2] = {0, 0}; /* !! The same length as `user_cd_nelem` specified !! */
  char name[16];
  for (size_t i = 0; i < 16; i++)
    name[i] = ' ';
  unsigned int flags = 0;
  herr_t status =
      H5Pget_filter_by_id(dcpl_id, H5Z_FILTER_SPERR, &flags, &user_cd_nelem, user_cd_values, 16,
                          name, user_cd_values + user_cd_nelem - 1);
  if (user_cd_nelem != 1) {
#ifndef NDEBUG
    printf("%s: %d, user_cd_nelem = %lu\n", __FILE__, __LINE__, user_cd_nelem);
#endif
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADSIZE,
            "User cd_values[] isn't a single element ??");
    return -1;
  }

  /* Get the datatype size. It must be 4 or 8, since the float type is verified by `can_apply`. */
  int is_float = 1;
  if (H5Tget_size(type_id) == 8)
    is_float = 0; /* !is_float, i.e. double */
  else
    assert(H5Tget_size(type_id) == 4);

  /* Get chunk sizes. */
  hsize_t chunks[4] = {0, 0, 0, 0};
  int ndims = H5Pget_chunk(dcpl_id, 4, chunks);
  int real_dims = 0;
  for (int i = 0; i < 4; i++)
    if (chunks[i] > 1)
      real_dims++;
  assert(real_dims == 2 || real_dims == 3);

  /*
   * Assemble the meta info to be stored.
   * [0]  : 2D/3D, float/double
   * [1]  : compression specifics
   * [2-3]: (dimx, dimy) in 2D cases.
   * [2-4]: (dimx, dimy, dimz) in 3D cases.
   */
  unsigned int cd_values[5] = {0, 0, 0, 0, 0};
  cd_values[0] = H5Z_SPERR_pack_data_type(real_dims, is_float);
  cd_values[1] = user_cd_values[0];
  int i1 = 2, i2 = 0;
  while (i2 < 4) {
    if (chunks[i2] > 1)
      cd_values[i1++] = (unsigned int)chunks[i2];
    i2++;
  }
  if (real_dims == 2)
    H5Pmodify_filter(dcpl_id, H5Z_FILTER_SPERR, H5Z_FLAG_MANDATORY, 4, cd_values);
  else
    H5Pmodify_filter(dcpl_id, H5Z_FILTER_SPERR, H5Z_FLAG_MANDATORY, 5, cd_values);

  return 0;
}

static size_t H5Z_filter_sperr(unsigned int flags,
                               size_t cd_nelmts,
                               const unsigned int cd_values[],
                               size_t nbytes,
                               size_t* buf_size,
                               void** buf)
{
  /* Extract info from cd_values[] */
  int rank = 0, is_float = 0;
  H5Z_SPERR_unpack_data_type(cd_values[0], &rank, &is_float);
  if ((rank == 2 && cd_nelmts != 4) || (rank == 3 && cd_nelmts != 5)) {
#ifndef NDEBUG
    printf("rank = %d, cd_nelmts = %lu\n", rank, cd_nelmts);
#endif
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADVALUE,
            "SPERR filter cd_values[] length not correct.");
    return 0;
  }

  int mode = 0, swap = 0;
  double quality = 0.0;
  H5Z_SPERR_decode_cd_values(cd_values[1], &mode, &quality, &swap);
  unsigned int dims[3] = {cd_values[2], cd_values[3], rank == 2 ? 1 : cd_values[4]};
  if (swap) {
    if (rank == 2) {
      unsigned int tmp = dims[0];
      dims[0] = dims[1];
      dims[1] = tmp;
    }
    else {
      unsigned int tmp = dims[0];
      dims[0] = dims[2];
      dims[2] = tmp;
    }
  }

  /* Decompression */
  if (flags & H5Z_FLAG_REVERSE) {
    void* dst = NULL; /* buffer to hold the decompressed data */
    int ret = 0;
    if (rank == 2)
      ret = sperr_decomp_2d(*buf, nbytes, is_float, dims[0], dims[1], &dst);
    else {
      size_t dimx = 0, dimy = 0, dimz = 0;
      ret = sperr_decomp_3d(*buf, nbytes, is_float, 1, &dimx, &dimy, &dimz, &dst);
    }
    if (ret != 0) {
      if (dst) {
        free(dst); /* allocated by SPERR, using malloc() */
        dst = NULL;
      }
      H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADVALUE,
              "SPERR decompression failed.");
      return 0;
    }

    size_t dst_len = (is_float ? 4ul : 8ul) * dims[0] * dims[1] * dims[2];
    if (dst_len <= *buf_size) { /* Re-use the input buffer */
      memcpy(*buf, dst, dst_len);
      free(dst); /* allocated by SPERR, using malloc() */
      dst = NULL;
    }
    else {                 /* Point to the new buffer */
      H5free_memory(*buf); /* allocated by HDF5 */
      *buf = dst;
      *buf_size = dst_len;
    }

    return dst_len;

  }      /* Finish Decompression */
  else { /* Compression */

    /* Sanity check on the data size. */
    if ((is_float ? 4ul : 8ul) * dims[0] * dims[1] * dims[2] != nbytes) {
      H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADSIZE,
              "Compression: input buffer len isn't right.");
      return 0;
    }

    void* dst = NULL; /* buffer to hold the compressed bitstream */
    size_t dst_len = 0;
    int ret = 0;

    if (rank == 2)
      ret = sperr_comp_2d(*buf, is_float, dims[0], dims[1], mode, quality, 0, &dst, &dst_len);
    else
      ret = sperr_comp_3d(*buf, is_float, dims[0], dims[1], dims[2], dims[0], dims[1], dims[2],
                          mode, quality, 1, &dst, &dst_len);
    if (ret != 0) {
      if (dst) {
        free(dst); /* allocated by SPERR, using malloc() */
        dst = NULL;
      }
      H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADVALUE,
              "SPERR compression failed.");
      return 0;
    }

    if (dst_len <= *buf_size) { /* Re-use the input buffer */
      memcpy(*buf, dst, dst_len);
      free(dst); /* allocated by SPERR, using malloc() */
      dst = NULL;
    }
    else {                 /* Point to the new buffer */
      H5free_memory(*buf); /* allocated by HDF5 */
      *buf = dst;
      *buf_size = dst_len;
    }

    return dst_len;

  } /* Finish compression */
}

const H5Z_class2_t H5Z_SPERR_class_t = {H5Z_CLASS_T_VERS, /* H5Z_class_t version */
                                        H5Z_FILTER_SPERR, /* Filter id number    */
                                        1,                /* encoder_present flag (set to true) */
                                        1,                /* decoder_present flag (set to true) */
                                        "H5Z-SPERR",      /* Filter name for debugging  */
                                        H5Z_can_apply_sperr, /* The "can apply" callback   */
                                        H5Z_set_local_sperr, /* The "set local" callback   */
                                        H5Z_filter_sperr};   /* The actual filter function */

const void* H5PLget_plugin_info()
{
  return &H5Z_SPERR_class_t;
}

H5PL_type_t H5PLget_plugin_type()
{
  return H5PL_TYPE_FILTER;
}
