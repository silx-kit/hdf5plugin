
#include <assert.h>
#include <stdlib.h>
#include "libQccPack.h"

int main(int argc, char** argv)
{
  if (argc != 6) {
    printf("Usage: ./a.out input_filename dim_x dim_y dim_z output_filename\n");
    return 1;
  }

  const char* input_name = argv[1];
  const size_t num_of_cols = atoi(argv[2]);
  const size_t num_of_rows = atoi(argv[3]);
  const size_t num_of_frms = atoi(argv[4]);
  const char* output_name = argv[5];
  const size_t num_of_vals = num_of_cols * num_of_rows * num_of_frms;

  /* allocate memory for a 1D array */
  float* buf = (float*)malloc(num_of_vals * sizeof(float));

  FILE* f = fopen(input_name, "r");
  size_t rnt = fread(buf, sizeof(float), num_of_vals, f);
  assert(rnt == num_of_vals);
  fclose(f);

  /* allocate data structure to take in this 1D array */
  QccWAVSubbandPyramid3D coeff3d;
  QccWAVSubbandPyramid3DInitialize(&coeff3d);
  coeff3d.num_cols = num_of_cols;
  coeff3d.num_rows = num_of_rows;
  coeff3d.num_frames = num_of_frms;
  QccWAVSubbandPyramid3DAlloc(&coeff3d);
  size_t counter = 0;
  for (size_t frame = 0; frame < num_of_frms; frame++)
    for (size_t row = 0; row < num_of_rows; row++)
      for (size_t col = 0; col < num_of_cols; col++)
        coeff3d.volume[frame][row][col] = buf[counter++];

  /* create a wavelet */
  QccString WaveletFilename = "CohenDaubechiesFeauveau.9-7.lft";
  QccString Boundary = "symmetric";
  QccWAVWavelet cdf97;
  if (QccWAVWaveletInitialize(&cdf97)) {
    QccErrorAddMessage("%s: Error calling QccWAVWaveletInitialize()", argv[0]);
    QccErrorExit();
  }
  if (QccWAVWaveletCreate(&cdf97, WaveletFilename, Boundary)) {
    QccErrorAddMessage("%s: Error calling QccWAVWaveletCreate()", argv[0]);
    QccErrorExit();
  }

  /* decide how many levels of transforms to perform */
  float min_xyz = (float)num_of_cols;
  if (num_of_rows < min_xyz)
    min_xyz = (float)num_of_rows;
  if (num_of_frms < min_xyz)
    min_xyz = (float)num_of_frms;
  float tmpf = log2f(min_xyz / 8.0f);
  int num_levels = tmpf < 0.0f ? 0 : (int)tmpf + 1;

  /* forward transform */
  QccWAVSubbandPyramid3DDWT(&coeff3d, QCCWAVSUBBANDPYRAMID3D_PACKET, num_levels, num_levels,
                            &cdf97);

  /* inverse transform */
  QccWAVSubbandPyramid3DInverseDWT(&coeff3d, &cdf97);

  /* copy result to an output buffer */
  float* out_buf = (float*)malloc(num_of_vals * sizeof(float));
  counter = 0;
  for (size_t frame = 0; frame < num_of_frms; frame++)
    for (size_t row = 0; row < num_of_rows; row++)
      for (size_t col = 0; col < num_of_cols; col++)
        out_buf[counter++] = coeff3d.volume[frame][row][col];

  /* write result to qccpack_result.raw */
  f = fopen(output_name, "w");
  fwrite(out_buf, sizeof(float), num_of_vals, f);
  fclose(f);

  /* free memory */
  free(out_buf);
  free(buf);
  QccWAVSubbandPyramid3DFree(&coeff3d);
}
