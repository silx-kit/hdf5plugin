/************************************************************

  Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
  https://blosc.org
  License: BSD 3-Clause (see LICENSE.txt)

  This test checks if HDF5 can read with Blosc2 filter
  a dataset plenty of Blosc2-compressed chunks:
  - Blosc2 with ZLIB (clevel 1) + SHUFFLE compression
  - Processing one chunk at a time.

 ************************************************************/

#include "hdf5.h"
#include "caterva.h"
#include "blosc2.h"
#include <stdio.h>
#include <stdlib.h>

#define FILE_CAT            "h5ex_cat.h5"
#define DATASET_CAT         "DSCAT"

int comp(char* urlpath_input)
{
    blosc_init();

    // Parameters definition
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_ctx_t *ctx;
    caterva_ctx_new(&cfg, &ctx);
    caterva_array_t *arr;
    caterva_open(ctx, urlpath_input, &arr);

    int8_t ndim = arr->ndim;
    int64_t *shape = arr->shape;
    int64_t extshape[8];
    int32_t *chunkshape = arr->chunkshape;
    int64_t *extchunkshape = arr->extchunkshape;
    hsize_t offset[8];
    int64_t chunksdim[8];
    int64_t nchunk_ndim[8];
    hsize_t chunks[8];
    int64_t chunknelems = 1;
    for (int i = 0; i < ndim; ++i) {
        offset[i] = nchunk_ndim[i] = 0;
        chunksdim[i] = (shape[i] - 1) / chunkshape[i] + 1;
        extshape[i] = extchunkshape[i] * chunksdim[i];
        chunknelems *= extchunkshape[i];
        chunks[i] = extchunkshape[i];
    }

    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.compcode = BLOSC_ZLIB;
    cparams.typesize = arr->itemsize;
    cparams.clevel = 1;
    cparams.nthreads = 6;
    cparams.blocksize = arr->sc->blocksize;
    blosc2_context *cctx;
    cctx = blosc2_create_cctx(cparams);

    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 6;
    blosc2_context *dctx;
    dctx = blosc2_create_dctx(dparams);

    int32_t chunksize = arr->sc->chunksize;
    uint8_t *chunk = malloc(chunksize);
    uint8_t *cchunk = malloc(chunksize);
    int8_t *buf_bypass = malloc(chunksize);
    int32_t *cbuffer = malloc(chunksize);
    int8_t *buf_filter = malloc(chunksize);

    int compressed, decompressed;
    int cat_cbytes, nbytes;
    cat_cbytes = nbytes = 0;

    hsize_t start[8],
            stride[8],
            count[8],
            block[8];

    hid_t           file_cat_w, file_cat_r, space, mem_space,
                    dset_cat_w, dset_cat_r, dcpl;    /* Handles */
    herr_t          status;
    unsigned        flt_msk = 0;

    // Create HDF5 dataset
    hid_t type_h5;
    switch (arr->itemsize) {
        case 1:
            type_h5 = H5T_STD_U8LE;
            break;
        case 4:
            type_h5 = H5T_STD_I32LE;
            break;
        case 8:
            type_h5 = H5T_STD_I64LE;
    }
    file_cat_w = H5Fcreate (FILE_CAT, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    space = H5Screate_simple (ndim, (const hsize_t *) extshape, NULL);
    hsize_t memsize = (hsize_t) chunknelems;
    dcpl = H5Pcreate (H5P_DATASET_CREATE);
    status = H5Pset_chunk (dcpl, ndim, chunks);

    // Enable Blosc2 filter
    unsigned int cd_values[7];
    /* 0 to 3 (inclusive) param slots are reserved. */
    cd_values[4] = 1;               /* compression level */
    cd_values[5] = 1;               /* 0: shuffle not active, 1: shuffle active */
    cd_values[6] = BLOSC_ZLIB;      /* the actual compressor to use */

    /* Set the filter with 7 params */
    int r = H5Pset_filter(dcpl, 32002, H5Z_FLAG_OPTIONAL, 7, cd_values);
    if(r<0) return -1;
    dset_cat_w = H5Dcreate (file_cat_w, DATASET_CAT, type_h5, space, H5P_DEFAULT, dcpl,
                            H5P_DEFAULT);

    for(int nchunk = 0; nchunk < arr->sc->nchunks; nchunk++) {
        // Get chunk offset
        blosc2_unidim_to_multidim((int8_t) ndim, (int64_t *) chunksdim, nchunk, (int64_t *) nchunk_ndim);
        for (int i = 0; i < ndim; ++i) {
            offset[i] = (hsize_t) nchunk_ndim[i] * extchunkshape[i];
        }

        // Get chunk
        decompressed = blosc2_schunk_decompress_chunk(arr->sc, nchunk, chunk, (int32_t) chunksize);
        if (decompressed < 0) {
            printf("Error reading chunk \n");
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        } else {
            nbytes += decompressed;
        }

        // Compress chunk using Blosc2 + ZLIB + SHUFFLE
        compressed = blosc2_compress_ctx(cctx, chunk, decompressed, cchunk, chunksize);
        if (compressed < 0) {
            printf("Error Caterva compress \n");
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        } else {
            cat_cbytes += compressed;
        }

        // Use H5Dwrite_chunk to save Blosc2 compressed buffer
        status = H5Dwrite_chunk(dset_cat_w, H5P_DEFAULT, flt_msk, offset, compressed, cchunk);
        if (status < 0) {
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        }
    }

    // Close and release resources.
    H5Pclose (dcpl);
    H5Sclose (space);
    H5Fclose (file_cat_w);
    H5Dclose (dset_cat_w);

    // Open HDF5 datasets
    file_cat_r = H5Fopen (FILE_CAT, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset_cat_r = H5Dopen (file_cat_r, DATASET_CAT, H5P_DEFAULT);
    dcpl = H5Dget_create_plist (dset_cat_r);
    space = H5Screate_simple (ndim, (const hsize_t *) extshape, NULL);
    mem_space = H5Screate_simple (1, &memsize, NULL);
    start[0] = 0;
    stride[0] = chunknelems;
    count[0] = 1;
    block[0] = chunknelems;
    status = H5Sselect_hyperslab (mem_space, H5S_SELECT_SET, start, stride, count, block);
    hsize_t cbufsize;

    for(int nchunk = 0; nchunk < arr->sc->nchunks; nchunk++) {
        // Get chunk offset
        blosc2_unidim_to_multidim((int8_t) ndim, (int64_t *) chunksdim, nchunk, (int64_t *) nchunk_ndim);
        for (int i = 0; i < ndim; ++i) {
            offset[i] = (hsize_t) nchunk_ndim[i] * extchunkshape[i];
        }

        // Read Blosc2 compressed chunk
        status = H5Dread_chunk(dset_cat_r, H5P_DEFAULT, offset, &flt_msk, cbuffer);
        if (status < 0) {
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        }

        // Decompress chunk using Blosc2 + ZLIB + SHUFFLE
        cbufsize = cbuffer[3];
        decompressed = blosc2_decompress_ctx(dctx, cbuffer, (int32_t) cbufsize, buf_bypass, chunksize);

        if (decompressed < 0) {
            printf("Error Caterva decompress \n");
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        }

        // Read chunk using HDF5 Blosc2 filter
        for (int i = 0; i < ndim; ++i) {
            start[i] = nchunk_ndim[i] * chunks[i];
            stride[i] = chunks[i];
            count[i] = 1;
            block[i] = chunks[i];
        }
        status = H5Sselect_hyperslab (space, H5S_SELECT_SET, start, stride, count,
                                      block);
        if (status < 0) {
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        }
        status = H5Dread(dset_cat_r, type_h5, mem_space, space, H5P_DEFAULT, buf_filter);
        if (status < 0) {
            free(chunk);
            free(cchunk);
            free(buf_bypass);
            free(cbuffer);
            free(buf_filter);
            return -1;
        }

        // Check that both buffers are equal
        for (int k = 0; k < decompressed / arr->itemsize; ++k) {
            if (buf_filter[k] != buf_bypass[k]) {
                printf("HDF5 with blosc2 filter output not equal to Blosc2 decompression output: %d, %d \n", buf_bypass[k], buf_filter[k]);
                return -1;
            }
        }
    }

    fprintf(stdout, "Success!\n");

    // Close and release resources.
    H5Sclose (space);
    H5Sclose (mem_space);
    H5Pclose (dcpl);
    H5Dclose (dset_cat_r);
    H5Fclose (file_cat_r);
    free(chunk);
    free(cchunk);
    free(buf_bypass);
    free(cbuffer);
    free(buf_filter);
    caterva_free(ctx, &arr);
    caterva_ctx_free(&ctx);

    blosc_destroy();

    return 0;
}


int solar1() {
    int result = comp("../data/solar1.cat");
    return result;
}

int air1() {
    int result = comp("../data/air1.cat");
    return result;
}

int snow1() {
    int result = comp("../data/snow1.cat");
    return result;
}

int wind1() {
    int result = comp("../data/wind1.cat");
    return result;
}

int precip1() {
    int result = comp("../data/precip1.cat");
    return result;
}

int precip2() {
    int result = comp("../data/precip2.cat");
    return result;
}

int precip3() {
    int result = comp("../data/precip3.cat");
    return result;
}

int precip3m() {
    int result = comp("../data/precip-3m.cat");
    return result;
}

int easy() {
    int result = comp("../data/easy.cat");
    return result;
}

int cyclic() {
    int result = comp("../data/cyclic.cat");
    return result;
}

int main() {

    unsigned majnum, minnum, vers;
    if (H5get_libversion(&majnum, &minnum, &vers) >= 0)
        printf("HDF5 working with version %d.%d.%d \n", majnum, minnum, vers);

    printf("cyclic \n");
    CATERVA_ERROR(cyclic());
    printf("easy \n");
    CATERVA_ERROR(easy());
    printf("wind1 \n");
    CATERVA_ERROR(wind1());
    printf("air1 \n");
    CATERVA_ERROR(air1());
    printf("solar1 \n");
    CATERVA_ERROR(solar1());
    printf("snow1 \n");
    CATERVA_ERROR(snow1());
    printf("precip1 \n");
    CATERVA_ERROR(precip1());
    /*  printf("precip2 \n");
    CATERVA_ERROR(precip2());
    printf("precip3 \n");
    CATERVA_ERROR(precip3());
    printf("precip3m \n");
    CATERVA_ERROR(precip3m());
    return CATERVA_SUCCEED;
*/
}
