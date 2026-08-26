#ifndef H5Z_HTJ2K_BACKEND_H
#define H5Z_HTJ2K_BACKEND_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define H5Z_HTJ2K_DTYPE(is_signed, nbytes)                                     \
  (((is_signed) ? 0x80 : 0x00) | (nbytes))
#define H5Z_HTJ2K_DTYPE_IS_SIGNED(dtype) ((dtype) & 0x80 ? true : false)
#define H5Z_HTJ2K_DTYPE_NBYTES(dtype) ((dtype) & 0x7F)

typedef enum {
  H5Z_HTJ2K_DTYPE_NONE = 0,
  H5Z_HTJ2K_DTYPE_UINT8 = H5Z_HTJ2K_DTYPE(false, 1),
  H5Z_HTJ2K_DTYPE_UINT16 = H5Z_HTJ2K_DTYPE(false, 2),
  H5Z_HTJ2K_DTYPE_INT8 = H5Z_HTJ2K_DTYPE(true, 1),
  H5Z_HTJ2K_DTYPE_INT16 = H5Z_HTJ2K_DTYPE(true, 2),
} H5Z_HTJ2K_DTYPE_t;

/**
 * h5z_htj2k_backend_compress() – encode a raw pixel buffer to a HTJ2K
 * codestream in memory.
 *
 * @param input_nbytes       byte length of the input pixel buffer
 * @param input_buffer       pointer to the input pixel buffer (interleaved
 * samples)
 * @param width              image width in pixels
 * @param height             image height in pixels
 * @param ncomps             number of components
 * @param dtype              sample type, one of H5Z_HTJ2K_DTYPE_t
 * (unsigned int)
 * @param num_threads        number of threads
 * @param output_nbytes      [out] byte length of the allocated output
 * codestream
 * @param output_buffer      [out] caller-owned output buffer (free() when
 * done)
 * @return 0 on success, -1 on failure
 */
int h5z_htj2k_backend_compress(size_t input_nbytes, void *input_buffer,
                               unsigned int width, unsigned int height,
                               unsigned int ncomps, unsigned int dtype,
                               int num_threads, size_t *output_nbytes,
                               void **output_buffer);

/**
 * h5z_htj2k_backend_decompress() – decode a HTJ2K codestream from memory.
 *
 * @param dtype              sample type without endianness bit, one of
 * H5Z_HTJ2K_DTYPE_t, use H5Z_HTJ2K_DTYPE_NONE to infer it from the
 * codestream
 * @param num_threads        number of threads
 * @param compressed_nbytes  byte length of the input codestream
 * @param compressed_buffer  pointer to the input codestream
 * @param output_nbytes      [out] byte length of the allocated output buffer
 * @param output_buffer      [out] caller-owned output buffer (free() when
 * done)
 * @param output_dtype       [out] Output sample type without endianness bit,
 * one of H5Z_HTJ2K_DTYPE_t
 * @return 0 on success, -1 on failure
 */
int h5z_htj2k_backend_decompress(unsigned int dtype, int num_threads,
                                 size_t compressed_nbytes,
                                 void *compressed_buffer, size_t *output_nbytes,
                                 void **output_buffer,
                                 unsigned int *output_dtype);

#ifdef __cplusplus
}
#endif

#endif