/*
   This program reads in a binary raw file, and output it in NetCDF format.
   The NetCDF file could be visualized in VisIt.
*/
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>

/* We are writing 2D data, an NX * NY grid. */
#define NDIMS 2
#define NX 128
#define NY 128

/* Handle errors by printing an error message and exiting with a non-zero
 * status. */
#define ERRCODE 2
#define ERR(e)                                      \
  {                                                 \
    free(data_buf);                                 \
    fprintf(stderr, "Error: %s\n", nc_strerror(e)); \
    exit(ERRCODE);                                  \
  }

int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: ./a.out input_filename output_filename.");
    return 1;
  }

  const long num_of_bytes = sizeof(float) * NX * NY;

  /* Check if the file to read is good */
  FILE* f = fopen(argv[1], "r");
  if (f == NULL) {
    fprintf(stderr, "Error! Cannot open input file: %s\n", argv[1]);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  if (ftell(f) < num_of_bytes) {
    fprintf(stderr, "Error! Input file size error: %s\n", argv[1]);
    fprintf(stderr, "  Expecting %ld bytes, got %ld bytes.\n", num_of_bytes, ftell(f));
    fclose(f);
    return 1;
  }
  fseek(f, 0, SEEK_SET);

  /* Allocate space for the input data */
  float* data_buf = (float*)malloc(num_of_bytes);
  if (fread(data_buf, sizeof(float), NX * NY, f) != NX * NY) {
    fprintf(stderr, "Error! Input file read error: %s\n", argv[1]);
    free(data_buf);
    fclose(f);
    return 1;
  }
  fclose(f);

  /* When we create netCDF variables and dimensions, we get back an ID for each
   * one. */
  int ncid, x_dimid, y_dimid, varid, retval;
  int dimids[NDIMS];

  /* Always check the return code of every netCDF function call. In
   * this example program, any retval which is not equal to NC_NOERR
   * (0) will cause the program to print an error message and exit
   * with a non-zero return code. */

  /* Create the file. The NC_CLOBBER parameter tells netCDF to
   * overwrite this file, if it already exists.*/
  if ((retval = nc_create(argv[2], NC_CLOBBER, &ncid)))
    ERR(retval);

  /* Define the dimensions. NetCDF will hand back an ID for each. */
  if ((retval = nc_def_dim(ncid, "x", NX, &x_dimid)))
    ERR(retval);
  if ((retval = nc_def_dim(ncid, "y", NY, &y_dimid)))
    ERR(retval);

  /* The dimids array is used to pass the IDs of the dimensions of the variable.
   */
  dimids[0] = x_dimid;
  dimids[1] = y_dimid;

  /* Define the variable. The type of the variable in this case is NC_FLOAT
   * (4-byte float). */
  if ((retval = nc_def_var(ncid, "data", NC_FLOAT, NDIMS, dimids, &varid)))
    ERR(retval);

  /* End define mode. This tells netCDF we are done defining metadata. */
  if ((retval = nc_enddef(ncid)))
    ERR(retval);

  /* Write the  data to the file. In this case we write all the data in one
   * operation. */
  if ((retval = nc_put_var_float(ncid, varid, data_buf)))
    ERR(retval);

  /* Close the file. This frees up any internal netCDF resources
   * associated with the file, and flushes any buffers. */
  if ((retval = nc_close(ncid)))
    ERR(retval);

  free(data_buf);

  printf("*** SUCCESS writing NetCDF file %s! ***\n", argv[2]);
  return 0;
}
