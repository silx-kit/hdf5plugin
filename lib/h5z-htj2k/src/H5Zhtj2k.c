/* HDF5 compression filter storing compressed chunk as HTJ2K (JPEG2000/part15)
 * code stream.
 */

#include "H5PLextern.h"
#include "H5Zhtj2k_backend.h"

#include <assert.h>
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define H5Z_HTJ2K_FILTER_ID 32033
#define H5Z_HTJ2K_VERSION 1

#define CD_INDEX_VERSION 0
#define CD_INDEX_DTYPE 1
#define CD_INDEX_WIDTH 2
#define CD_INDEX_HEIGHT 3
#define CD_INDEX_NCOMPS 4
#define CD_NELMTS_COMPRESS_REQUIRED 5

#if defined(_MSC_VER)
#define NATIVE_ORDER H5T_ORDER_LE
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define NATIVE_ORDER H5T_ORDER_BE
#else
#define NATIVE_ORDER H5T_ORDER_LE
#endif

#define H5Z_HTJ2K_DTYPE_WITH_ENDIANNESS(order, is_signed, nbytes)              \
  (((order) == H5T_ORDER_BE && nbytes > 1 ? 0x0100 : 0x0000) |                 \
   H5Z_HTJ2K_DTYPE(is_signed, nbytes))
#define H5Z_HTJ2K_DTYPE_WITHOUT_ENDIANNESS(dtype_with_endianness)              \
  ((dtype_with_endianness) & 0x00FF)
#define H5Z_HTJ2K_DTYPE_ORDER(dtype_with_endianness)                           \
  (H5Z_HTJ2K_DTYPE_NBYTES(dtype_with_endianness) == 1 ? H5T_ORDER_NONE         \
   : (dtype_with_endianness) & 0x0100                 ? H5T_ORDER_BE           \
                                                      : H5T_ORDER_LE)

static herr_t set_local_htj2k(hid_t dcpl, hid_t type, hid_t space);
static size_t filter_htj2k(unsigned int flags, size_t cd_nelmts,
                           const unsigned int cd_values[], size_t nbytes,
                           size_t *buf_size, void **buf);

const H5Z_class2_t H5Z_HTJ2K[1] = {{
    H5Z_CLASS_T_VERS,                  /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_HTJ2K_FILTER_ID, /* Filter id number             */
                                       /* encoder_present flag (set to true) */
#ifdef H5Z_HTJ2K_FILTER_DECODE_ONLY
    0,
#else
    1,
#endif
    1,       /* decoder_present flag (set to true) */
    "htj2k", /* Filter name for debugging    */
    NULL,    /* The "can apply" callback     */
#ifdef H5Z_HTJ2K_FILTER_DECODE_ONLY
    NULL,
#else
    (H5Z_set_local_func_t)set_local_htj2k,
#endif
    (H5Z_func_t)filter_htj2k,
}};
// TODO add can_apply

H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }

const void *H5PLget_plugin_info(void) { return H5Z_HTJ2K; }

#ifndef H5Z_HTJ2K_FILTER_DECODE_ONLY
static herr_t set_local_htj2k(hid_t dcpl, hid_t type, hid_t space) {
  herr_t result;
  unsigned int flags;
  size_t input_cd_nelmts = 0;
  size_t cd_nelmts = CD_NELMTS_COMPRESS_REQUIRED;
  unsigned cd_values[CD_NELMTS_COMPRESS_REQUIRED] = {0};

  result = H5Pget_filter_by_id2(dcpl, H5Z_HTJ2K_FILTER_ID, &flags,
                                &input_cd_nelmts, NULL, 0, NULL, NULL);
  if (result < 0) {
    fprintf(stderr, "H5Pget_filter_by_id2 failed\n");
    return -1;
  }

  cd_values[CD_INDEX_VERSION] = H5Z_HTJ2K_VERSION;

  /* Retrieve width, height, number of components */
  int ndims;
  hsize_t dims[H5S_MAX_RANK], dims_used[H5S_MAX_RANK];

  ndims = H5Sget_simple_extent_dims(space, dims, NULL);
  if (ndims < 0) {
    fprintf(stderr, "H5Sget_simple_extent_dims failed\n");
    return -1;
  }

  /* compute non-unity dimensions in chunk */
  unsigned int ndims_used = 0;
  unsigned int dim_index;
  for (dim_index = 0; dim_index < (unsigned int)ndims; dim_index++) {
    if (dims[dim_index] <= 1)
      continue;
    dims_used[ndims_used] = dims[dim_index];
    ndims_used++;
  }
  if (ndims_used == 0) {
    cd_values[CD_INDEX_WIDTH] = 1;
    cd_values[CD_INDEX_HEIGHT] = 1;
    cd_values[CD_INDEX_NCOMPS] = 1;
  } else if (ndims_used == 1) {
    if (dims[ndims - 1] <= 1) {
      cd_values[CD_INDEX_WIDTH] = 1;
      cd_values[CD_INDEX_HEIGHT] = dims_used[0];
      cd_values[CD_INDEX_NCOMPS] = 1;
    } else {
      cd_values[CD_INDEX_WIDTH] = dims_used[0];
      cd_values[CD_INDEX_HEIGHT] = 1;
      cd_values[CD_INDEX_NCOMPS] = 1;
    }
  } else if (ndims_used == 2) {
    cd_values[CD_INDEX_WIDTH] = dims_used[1];
    cd_values[CD_INDEX_HEIGHT] = dims_used[0];
    cd_values[CD_INDEX_NCOMPS] = 1;
  } else if (ndims_used == 3) {
    if (dims_used[2] != 3) {
      fprintf(stderr, "For chunks with 3 non-unity dimensions, the last "
                      "dimension must be equal to 3\n");
      return -1;
    }
    cd_values[CD_INDEX_WIDTH] = dims_used[1];
    cd_values[CD_INDEX_HEIGHT] = dims_used[0];
    cd_values[CD_INDEX_NCOMPS] = dims_used[2];
  } else {
    fprintf(stderr, "Unsupported number of dimensions\n");
    return -1;
  }

  /* Retrieve data type */
  int dtype = 0;
  H5T_class_t dclass;
  dclass = H5Tget_class(type);
  if (dclass < 0) {
    fprintf(stderr, "H5Tget_class failed\n");
    return -1;
  }

  if (dclass != H5T_INTEGER) {
    fprintf(stderr, "Unsupported datatype class: %d\n", dclass);
    return -1;
  }

  size_t dsize;
  dsize = H5Tget_size(type);
  if (dsize == 0) {
    fprintf(stderr, "H5Tget_size failed\n");
    return -1;
  }
  if (dsize != sizeof(uint8_t) && dsize != sizeof(uint16_t)) {
    fprintf(stderr, "Unsupported datatype size\n");
    return -1;
  }

  H5T_order_t order = H5Tget_order(type);
  if (order == H5T_ORDER_ERROR) {
    fprintf(stderr, "H5Tget_order failed\n");
    return -1;
  }
  if (dsize > 1 && order != H5Tget_order(H5T_NATIVE_INT16)) {
    fprintf(stderr,
            "Unsupported byte order (only native byte order is supported)\n");
    return -1;
  }

  H5T_sign_t sign;
  sign = H5Tget_sign(type);
  if (sign == H5T_SGN_ERROR) {
    fprintf(stderr, "H5Tget_sign failed\n");
    return -1;
  }

  dtype = H5Z_HTJ2K_DTYPE_WITH_ENDIANNESS(order, sign == H5T_SGN_2, dsize);
  cd_values[CD_INDEX_DTYPE] = dtype;

  result =
      H5Pmodify_filter(dcpl, H5Z_HTJ2K_FILTER_ID, flags, cd_nelmts, cd_values);
  if (result < 0) {
    fprintf(stderr, "H5Pmodify_filter failed\n");
    return -1;
  }

  return 1;
}
#endif /* H5Z_HTJ2K_FILTER_DECODE_ONLY */

static int env_int(const char *name, int default_value) {
  const char *v = getenv(name);
  if (v == NULL || *v == '\0') {
    return default_value;
  }
  char *end = NULL;
  long n = strtol(v, &end, 10);
  if (end == v || (end != NULL && *end != '\0')) {
    return default_value;
  }
  if (n < 0) {
    n = 0;
  } else if (n > 1024) {
    n = 1024;
  }
  return (int)n;
}

static int check_j2k_codestream_magic(const void *input_buffer, size_t nbytes) {
  unsigned char magic[2];

  if (nbytes < 2) {
    return -1;
  }
  memcpy(magic, input_buffer, 2);
  if (magic[0] != 0xff || magic[1] != 0x4f) {
    return -1;
  }
  return 0;
}

static size_t filter_htj2k(unsigned int flags, size_t cd_nelmts,
                           const unsigned int cd_values[], size_t nbytes,
                           size_t *buf_size, void **buf) {
  int result;
  size_t output_size;
  void *output_buffer = NULL;
  void *input_buffer = *buf;
  unsigned int version =
      cd_nelmts > CD_INDEX_VERSION ? cd_values[CD_INDEX_VERSION] : 0;
  unsigned int dtype_with_endianness = cd_nelmts > CD_INDEX_DTYPE
                                           ? cd_values[CD_INDEX_DTYPE]
                                           : H5Z_HTJ2K_DTYPE_NONE;
  unsigned int dtype =
      H5Z_HTJ2K_DTYPE_WITHOUT_ENDIANNESS(dtype_with_endianness);

  if (version > H5Z_HTJ2K_VERSION) {
    fprintf(stderr, "Incompatible HTJ2K filter version\n");
    return 0;
  }

  int num_threads = env_int("H5Z_HTJ2K_NUM_THREADS", 0);

  if (flags & H5Z_FLAG_REVERSE) { /** Decompress data **/
    if (check_j2k_codestream_magic(input_buffer, nbytes) < 0) {
      fprintf(stderr, "Invalid J2K codestream magic\n");
      return 0;
    }

    unsigned int output_dtype = H5Z_HTJ2K_DTYPE_NONE;
    result = h5z_htj2k_backend_decompress(dtype, num_threads, nbytes,
                                          input_buffer, &output_size,
                                          &output_buffer, &output_dtype);
    if (result < 0) {
      fprintf(stderr, "decompress failed\n");
      if (output_buffer) {
        free(output_buffer);
      }
      return 0;
    }

    unsigned int effective_dtype =
        dtype != H5Z_HTJ2K_DTYPE_NONE ? dtype : output_dtype;
    size_t effective_nbytes = H5Z_HTJ2K_DTYPE_NBYTES(effective_dtype);
    if (effective_nbytes != 1 && effective_nbytes != 2) {
      fprintf(stderr, "decompress failed: unable to resolve data type\n");
      if (output_buffer) {
        free(output_buffer);
      }
      return 0;
    }

    /* Decompressed data is always in native byte order.
       If the data type is not in native byte order,
       we need to swap the bytes since hdf5 does not expect that. */
    if (effective_nbytes != 1 &&
        H5Z_HTJ2K_DTYPE_ORDER(dtype_with_endianness) != NATIVE_ORDER) {
      size_t nelemts = output_size / effective_nbytes;
      uint16_t *data = (uint16_t *)output_buffer;
      for (size_t i = 0; i < nelemts; i++) {
        uint16_t v = data[i];
        data[i] = (uint16_t)((v << 8) | (v >> 8));
      }
    }

  } else { /** Compress data **/
#ifdef H5Z_HTJ2K_FILTER_DECODE_ONLY
    fprintf(stderr, "compression not available\n");
    return 0;
#else
    if (cd_nelmts < CD_NELMTS_COMPRESS_REQUIRED) {
      fprintf(stderr, "Missing some cd_values for compression\n");
      return 0;
    }

    result = h5z_htj2k_backend_compress(
        nbytes, input_buffer, cd_values[CD_INDEX_WIDTH],
        cd_values[CD_INDEX_HEIGHT], cd_values[CD_INDEX_NCOMPS], dtype,
        num_threads, &output_size, &output_buffer);
    if (result < 0) {
      fprintf(stderr, "compress failed\n");
      if (output_buffer) {
        free(output_buffer);
      }
      return 0;
    }
#endif /* H5Z_HTJ2K_FILTER_DECODE_ONLY */
  }

  free(input_buffer);
  *buf = output_buffer;
  *buf_size = output_size;
  return output_size;
}
