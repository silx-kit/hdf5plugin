/*
   This program reads in a 2D binary raw file of dimension (IN_NX, IN_NY),
   crops the corner with dimension (OUT_NX, OUT_NY), and outputs the corner.
*/
#include <stdio.h>
#include <stdlib.h>

#define IN_NX 128
#define IN_NY 128
#define OUT_NX 15
#define OUT_NY 15

int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: ./a.out input_filename output_filename.\n");
    return 1;
  }

  const long in_num_of_vals = IN_NX * IN_NY;
  const long in_num_of_bytes = in_num_of_vals * sizeof(float);
  const long out_num_of_vals = OUT_NX * OUT_NY;
  const long out_num_of_bytes = out_num_of_vals * sizeof(float);

  /* Check if the file to read is good */
  FILE* f = fopen(argv[1], "r");
  if (f == NULL) {
    fprintf(stderr, "Error! Cannot open input file: %s\n", argv[1]);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  if (ftell(f) != in_num_of_bytes) {
    fprintf(stderr, "Error! Input file size error: %s\n", argv[1]);
    fprintf(stderr, "  Expecting %ld bytes, got %ld bytes.\n", in_num_of_bytes, ftell(f));
    fclose(f);
    return 1;
  }
  fseek(f, 0, SEEK_SET);

  /* Allocate space for the input data */
  float* in_buf = (float*)malloc(in_num_of_bytes);
  if (fread(in_buf, sizeof(float), in_num_of_vals, f) != in_num_of_vals) {
    fprintf(stderr, "Error! Input file read error: %s\n", argv[1]);
    free(in_buf);
    fclose(f);
    return 1;
  }
  fclose(f);

  /* Allocate space for the output data, and fill it up. */
  float* out_buf = (float*)malloc(out_num_of_bytes);
  long x, y, counter = 0;
  for (y = 0; y < OUT_NY; y++)
    for (x = 0; x < OUT_NX; x++) {
      long in_idx = y * IN_NX + x;
      out_buf[counter++] = in_buf[in_idx];
    }

  /* Write out_buf to a file. */
  f = fopen(argv[2], "w");
  if (f == NULL) {
    fprintf(stderr, "Error! Cannot open output file: %s\n", argv[2]);
    free(in_buf);
    free(out_buf);
    return 1;
  }
  if (fwrite(out_buf, sizeof(float), out_num_of_vals, f) != out_num_of_vals) {
    fprintf(stderr, "Error! Input file write error: %s\n", argv[2]);
    free(in_buf);
    free(out_buf);
    fclose(f);
    return 1;
  }
  fclose(f);
  free(in_buf);
  free(out_buf);

  printf("*** SUCCESS cropping %s to %s! ***\n", argv[1], argv[2]);
  return 0;
}
