#include <stdio.h>
#include <stdlib.h>
#include "h5z-sperr.h"

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: ./generate_cd_values  compression_mode  compression_quality\n");
    exit(1);
  }

  int mode = atoi(argv[1]);
  double quality = atof(argv[2]);
  unsigned int cd_values = H5Z_SPERR_make_cd_values(mode, quality);

  switch (mode) {
    case 1:
      if (quality > 0.0 && quality < 64.0)
        printf("For fixed-rate compression with a bitrate of %.f,\n", quality);
      else {
        printf("Target bitrate should in between of 0.0 and 64.0\n");
        exit(1);
      }
      break;
    case 2:
      if (quality > 0.0) 
        printf("For fixed-PSNR compression with a target PSNR of %.f,\n", quality);
      else {
        printf("Target PSNR should be greater than 0.0.\n");
        exit(1);
      }
      break;
    case 3:
      if (quality > 0.0) 
        printf("For fixed-PWE compression with a PWE tolerance of %.f,\n", quality);
      else {
        printf("PWE tolerance should be greater than 0.0.\n");
        exit(1);
      }
      break;
    default:
      printf("Compression mode should be 1, 2, or 3\n");
        exit(1);
  }
  
  printf("H5Z-SPERR cd_values = %uu.\n", cd_values);
  printf("Please use this value as a single 32-bit unsigned integer in your applications.\n");
}
