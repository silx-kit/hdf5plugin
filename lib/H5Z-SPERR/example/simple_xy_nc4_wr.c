/* Copyright 2019 University Corporation for Atmospheric
   Research/Unidata.  See COPYRIGHT file for conditions of use. */
/**
 * @file
 * Write the simple_xy file, with some of the features of netCDF-4.
 *
 * This is a very simple example which is based on the simple_xy
 * example, but which uses netCDF-4 features, such as
 * compression. Please see the simple_xy example to learn more about
 * the netCDF-3 API.
 *
 * Like simple_xy_wr.c, this program writes a 2D netCDF variable
 * (called "data") and fills it with sample data.  It has two
 * dimensions, "x" and "y".
 *
 * Full documentation for netCDF can be found at:
 * https://docs.unidata.ucar.edu/netcdf-c.
 *
 * @author Ed Hartnett
*/
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

/*
 * Include this header so SPERR compression parameters can be encoded.
 */
#include "../include/h5z-sperr.h"

/* We are writing 2D data, a 12 x 12 grid. */
#define NDIMS 2
#define NX 12
#define NY 12

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

int
main()
{
   int ncid, x_dimid, y_dimid, varid;
   int dimids[NDIMS];
   size_t chunks[NDIMS];
   int deflate = 1, deflate_level = 1;
   float data_out[NX][NY];
   int x, y, retval;

   /* Create some pretend data. If this wasn't an example program, we
    * would have some real data to write, for example, model output. */
   for (x = 0; x < NX; x++)
      for (y = 0; y < NY; y++)
         data_out[x][y] = (float)x * NY + y;

   /* Create the file. The NC_NETCDF4 parameter tells netCDF to create
    * a file in netCDF-4/HDF5 standard. */
   if ((retval = nc_create("simple_xy_nc4.nc", NC_NETCDF4, &ncid)))
      ERR(retval);

   /* Define the dimensions. */
   if ((retval = nc_def_dim(ncid, "x", NX, &x_dimid)))
      ERR(retval);
   if ((retval = nc_def_dim(ncid, "y", NY, &y_dimid)))
      ERR(retval);

   /* Set up variable data. */
   dimids[0] = x_dimid;
   dimids[1] = y_dimid;
   chunks[0] = NX;
   chunks[1] = NY;

   /* Define the variable. */
   if ((retval = nc_def_var(ncid, "data", NC_FLOAT, NDIMS, dimids, &varid)))
      ERR(retval);
   if ((retval = nc_def_var_chunking(ncid, varid, 0, &chunks[0])))
      ERR(retval);

    /*
     * Encode SPERR compression parameters:
     * 1 == fixed bitrate mode 
     * 6.4 == target bitrate
     * 0 == no rank swaps
     * 32028 == registered ID for the SPERR filter.
     */
    unsigned int cd_values = H5Z_SPERR_make_cd_values(1, 6.4, 0);
    if ((retval = nc_def_var_filter(ncid, varid, 32028, 1, &cd_values)))
      ERR(retval);

   /* 
    * One can also apply another lossless filter, but please make sure that
    * 1) they are only applied AFTER the SPERR filter, and
    * 2) the byte shuffle feature is turned off (NC_NOSHUFFLE).
    */
   if ((retval = nc_def_var_deflate(ncid, varid, NC_NOSHUFFLE, deflate, deflate_level)))
      ERR(retval);

   /* No need to explicitly end define mode for netCDF-4 files. Write
    * the pretend data to the file. */
   if ((retval = nc_put_var_float(ncid, varid, &data_out[0][0])))
      ERR(retval);

   /* Close the file. */
   if ((retval = nc_close(ncid)))
      ERR(retval);

   printf("*** SUCCESS writing example file simple_xy_nc4.nc!\n");
   return 0;
}
