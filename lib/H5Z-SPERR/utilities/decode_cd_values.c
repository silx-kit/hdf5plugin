#include <stdio.h>
#include <stdlib.h>
#include "h5z-sperr.h"

int main(int argc, char* argv[])
{
  if (argc != 2) {
    printf("Usage: ./decode_cd_values  cd_values\n");
    exit(1);
  }

  long int cd_values = atol(argv[1]);

  int mode = 0;
  double quality = 0.0;
  int swap = 0;

  H5Z_SPERR_decode_cd_values((unsigned int)cd_values, &mode, &quality, &swap);

  printf("H5Z-SPERR cd_values %uu ", (unsigned int)cd_values);
  switch (mode) {
    case 1:
      printf("means fixed-rate compression with a bitrate of %.4f, ", quality);
      break;
    case 2:
        printf("means fixed-PSNR compression with a target PSNR of %.4f, ", quality);
      break;
    case 3:
        printf("means fixed-PWE compression with a PWE tolerance of %.4g, ", quality);
      break;
    default:
        exit(1);
  }

  if (swap)
    printf("swapping rank orders.\n");
  else
    printf("without swapping rank orders.\n");
}
