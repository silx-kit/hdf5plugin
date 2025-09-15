#ifndef H5Z_SPERR_H
#define H5Z_SPERR_H

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define H5Z_FILTER_SPERR 32028

#define FRACTIONAL_BITS 16
#define INTEGER_BITS 12

/*
 * This function encodes 1) the SPERR compression mode, 2) compression quality, 3) if to swap
 * rank orders into a 32-bit unsigned int. Valid input and its meaning:
 *   - Fixed bitrate compression:                mode = 1; quality = target bitrate.
 *   - Fixed PSNR compression:                   mode = 2; quality = target PSNR.
 *   - Fixed PWE (point-wise error) compression: mode = 3; quality = PWE tolerance.
 *   - In all modes, swap != 0 means to perform rank order swaps.
 * The encoded value is returned and needs to be passed to HDF5 as `cd_values[1]`.
 */
unsigned int H5Z_SPERR_make_cd_values(int mode, double quality, int swap)
{
  assert(1 <= mode && mode <= 3);
  assert(quality > 0.0);

  unsigned int ret = 0;

  if (mode == 1 || mode == 2) {
    quality = round(quality * (1u << FRACTIONAL_BITS));
    ret = (unsigned int)quality;
  }
  else if (mode == 3) {
    /*
     *Use the logarithm of quality, and also encode its sign.
     */
    quality = log2(quality);

    int negative = 0;
    if (quality < 0.0) {
      negative = 1;
      quality *= -1.0;
      quality = ceil(quality * (1u << FRACTIONAL_BITS));
    }
    else
      quality = floor(quality * (1u << FRACTIONAL_BITS));

    ret = (unsigned int)quality;
    if (negative)
      ret |= 1u << (INTEGER_BITS + FRACTIONAL_BITS - 1);
  }

  /* Encode mode in bit 28, 29, and 30. */
  unsigned int mask = 0;
  switch (mode) {
    case 1:
      mask |= 1u << (INTEGER_BITS + FRACTIONAL_BITS);
      break;
    case 2:
      mask |= 1u << (INTEGER_BITS + FRACTIONAL_BITS + 1);
      break;
    case 3:
      mask |= 1u << (INTEGER_BITS + FRACTIONAL_BITS);
      mask |= 1u << (INTEGER_BITS + FRACTIONAL_BITS + 1);
      break;
    default:;
  }
  ret |= mask;

  /* Encode swap flag at bit 31. */
  if (swap)
    ret |= 1u << (INTEGER_BITS + FRACTIONAL_BITS + 3);

  return ret;
}

void H5Z_SPERR_decode_cd_values(unsigned int cd_val, /* input */
                                int* mode,           /* output */
                                double* quality,     /* output */
                                int* swap)           /* output */
{
  /* Decode the rank swap flag. */
  *swap = cd_val >> (INTEGER_BITS + FRACTIONAL_BITS + 3);

  /* Decode the compression mode from the next 3 bits */
  unsigned int bit1 = (cd_val >> (INTEGER_BITS + FRACTIONAL_BITS)) & 1u;
  unsigned int bit2 = (cd_val >> (INTEGER_BITS + FRACTIONAL_BITS + 1)) & 1u;
  if (bit1 && !bit2)
    *mode = 1;
  else if (!bit1 && bit2)
    *mode = 2;
  else if (bit1 && bit2)
    *mode = 3;

  int negative = 0;
  if ((cd_val >> (INTEGER_BITS + FRACTIONAL_BITS - 1)) & 1u)
    negative = 1;

  /* Zero out the top 4 bits and the sign bit (5 bits in total) */
  unsigned int mask = 1u << (INTEGER_BITS + FRACTIONAL_BITS - 1);
  cd_val &= (mask - 1);

  *quality = (double)cd_val / (double)(1u << FRACTIONAL_BITS);
  if (negative)
    *quality *= -1.0;

  if (*mode == 3)
    *quality = exp2(*quality);
}

#endif
