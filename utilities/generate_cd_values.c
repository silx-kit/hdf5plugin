#include <stdio.h>
#include <stdlib.h>
#include "h5z-sperr.h"

int main(int argc, char* argv[])
{
  if (argc < 3 || argc > 4) {
    printf("Usage: ./generate_cd_values  compression_mode  compression_quality  [rank_swap_flag]\n");
    exit(1);
  }

  int mode = atoi(argv[1]);
  double quality = atof(argv[2]);
  int swap = argc == 3 ? 0 : 1;
  unsigned int cd_values = H5Z_SPERR_make_cd_values(mode, quality, swap);

  H5Z_SPERR_decode_cd_values(cd_values, &mode, &quality, &swap);

  switch (mode) {
    case 1:
      if (quality > 0.0 && quality < 64.0)
        printf("For fixed-rate compression with a bitrate of %.4f,", quality);
      else {
        printf("Target bitrate should in between of 0.0 and 64.0\n");
        exit(1);
      }
      break;
    case 2:
      if (quality > 0.0) 
        printf("For fixed-PSNR compression with a target PSNR of %.4f,", quality);
      else {
        printf("Target PSNR should be greater than 0.0.\n");
        exit(1);
      }
      break;
    case 3:
      if (quality > 0.0) 
        printf("For fixed-PWE compression with a PWE tolerance of %.4g,", quality);
      else {
        printf("PWE tolerance should be greater than 0.0.\n");
        exit(1);
      }
      break;
    default:
      printf("Compression mode should be 1, 2, or 3\n");
        exit(1);
  }

  if (swap)
    printf(" swapping rank orders,\n");
  else
    printf(" without swapping rank orders,\n");
  printf("H5Z-SPERR cd_values = %uu (Filter ID = %d).\n", cd_values, H5Z_FILTER_SPERR);
  printf("Please use this value as a single 32-bit unsigned integer in your applications.\n");
}
