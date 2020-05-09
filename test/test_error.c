/*
Copyright (c) 2016, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Mark C. Miller, miller86@llnl.gov
LLNL-CODE-707197. All rights reserved.

This file is part of H5Z-ZFP. Please also read the BSD license
https://raw.githubusercontent.com/LLNL/H5Z-ZFP/master/LICENSE 
*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <string.h>

#include "hdf5.h"

#ifdef H5Z_ZFP_USE_PLUGIN
#include "H5Zzfp_plugin.h"
#else
#include "H5Zzfp_lib.h"
#include "H5Zzfp_props.h"
#endif

#define NAME_LEN 256

/* convenience macro to handle command-line args and help */
#define HANDLE_SEP(SEPSTR)                                      \
{                                                               \
    char tmpstr[64];                                            \
    int len = snprintf(tmpstr, sizeof(tmpstr), "\n%s...", #SEPSTR);\
    printf("    %*s\n",60-len,tmpstr);                          \
}

#define HANDLE_ARG(A,PARSEA,PRINTA,HELPSTR)                     \
{                                                               \
    int i;                                                      \
    char tmpstr[64];                                            \
    int len;                                                    \
    int len2 = strlen(#A)+1;                                    \
    for (i = 0; i < argc; i++)                                  \
    {                                                           \
        if (!strncmp(argv[i], #A"=", len2))                     \
        {                                                       \
            A = PARSEA;                                         \
            break;                                              \
        }                                                       \
        else if (!strncasecmp(argv[i], "help", 4))              \
        {                                                       \
            return 0;                                           \
        }                                                       \
    }                                                           \
    len = snprintf(tmpstr, sizeof(tmpstr), "%s=" PRINTA, #A, A);\
    printf("    %s%*s\n",tmpstr,60-len,#HELPSTR);               \
}


/* convenience macro to handle errors */
#define ERROR(FNAME)                                              \
do {                                                              \
    int _errno = errno;                                           \
    fprintf(stderr, #FNAME " failed at line %d, errno=%d (%s)\n", \
        __LINE__, _errno, _errno?strerror(_errno):"ok");          \
    return 1;                                                     \
} while(0)

/* Generate a simple, 1D sinusioidal data array with some noise */
#define TYPINT 1
#define TYPDBL 2
static int gen_data(size_t npoints, double noise, double amp, void **_buf, int typ)
{
    size_t i;
    double *pdbl = 0;
    int *pint = 0;

    /* create data buffer to write */
    if (typ == TYPINT)
        pint = (int *) malloc(npoints * sizeof(int));
    else
        pdbl = (double *) malloc(npoints * sizeof(double));
    srandom(0xDeadBeef);
    for (i = 0; i < npoints; i++)
    {
        double x = 2 * M_PI * (double) i / (double) (npoints-1);
        double n = noise * ((double) random() / ((double)(1<<31)-1) - 0.5);
        if (typ == TYPINT)
            pint[i] = (int) (amp * (1 + sin(x)) + n);
        else
            pdbl[i] = (double) (amp * (1 + sin(x)) + n);
    }
    if (typ == TYPINT)
        *_buf = pint;
    else
        *_buf = pdbl;
    return 0;
}

static hid_t setup_filter(int n, hsize_t *chunk, int zfpmode,
    double rate, double acc, uint prec,
    uint minbits, uint maxbits, uint maxprec, int minexp)
{
    hid_t cpid;
    unsigned int cd_values[10];
    int i, cd_nelmts = 10;

    /* setup dataset creation properties */
    if (0 > (cpid = H5Pcreate(H5P_DATASET_CREATE))) ERROR(H5Pcreate);
    if (0 > H5Pset_chunk(cpid, n, chunk)) ERROR(H5Pset_chunk);

    /* When filter is used as a library, we need to init it */
    H5Z_zfp_initialize();

    /* Setup the filter using properties interface. These calls also add
       the filter to the pipeline */
    if (zfpmode == H5Z_ZFP_MODE_RATE)
        H5Pset_zfp_rate(cpid, rate);
    else if (zfpmode == H5Z_ZFP_MODE_PRECISION)
        H5Pset_zfp_precision(cpid, prec);
    else if (zfpmode == H5Z_ZFP_MODE_ACCURACY)
        H5Pset_zfp_accuracy(cpid, acc);
    else if (zfpmode == H5Z_ZFP_MODE_EXPERT)
        H5Pset_zfp_expert(cpid, minbits, maxbits, maxprec, minexp);
    else if (zfpmode == H5Z_ZFP_MODE_REVERSIBLE)
        H5Pset_zfp_reversible(cpid);

    return cpid;
}

typedef struct client_data {char const *str; int has_str;} client_data_t;

static int walk_hdf5_error_stack_cb(unsigned int n, H5E_error_t const *err_desc, void *_cd)
{   
    client_data_t *cd  = (client_data_t *) _cd;
    if (n > 0) return 0;
    cd->has_str = strcasestr(err_desc->desc, cd->str) != 0;
    return 0;
}

static int check_hdf5_error_stack_for_string(char const *str)
{
    client_data_t cd = {str, 0};
    H5Ewalk(H5E_DEFAULT, H5E_WALK_UPWARD, walk_hdf5_error_stack_cb, &cd);
    return cd.has_str;
}

#define DSIZE 2048
#define FNAME "test_zfp_errors.h5"

int main(int argc, char **argv)
{
    int i, fd, ndiffs;
    unsigned corrupt[4] = {0xDeadBeef,0xBabeFace, 0xDeadBabe, 0xBeefFace};
    double d = 1.0, *buf = 0, rbuf[DSIZE];
    hsize_t chunk[] = {DSIZE,16,16,16,16};
    haddr_t off;
    hsize_t siz;

    /* HDF5 related variables */
    hid_t fid, tid, dsid, idsid, sid, cpid;

    int help = 0;

    /* compression parameters (defaults taken from ZFP header) */
    int zfpmode = H5Z_ZFP_MODE_ACCURACY;
    double rate = 4;
    double acc = 0.1;
    uint prec = 11;
    uint minbits = 0;
    uint maxbits = 4171;
    uint maxprec = 64;
    int minexp = -1074;

    /* ZFP filter arguments */
    HANDLE_SEP(ZFP compression paramaters)
    HANDLE_ARG(zfpmode,(int) strtol(argv[i]+len2,0,10),"%d", (1=rate,2=prec,3=acc,4=expert,5=reversible)); 
    HANDLE_ARG(rate,(double) strtod(argv[i]+len2,0),"%g",set rate for rate mode);
    HANDLE_ARG(acc,(double) strtod(argv[i]+len2,0),"%g",set accuracy for accuracy mode);
    HANDLE_ARG(prec,(uint) strtol(argv[i]+len2,0,10),"%u",set precision for precision mode);
    HANDLE_ARG(minbits,(uint) strtol(argv[i]+len2,0,10),"%u",set minbits for expert mode);
    HANDLE_ARG(maxbits,(uint) strtol(argv[i]+len2,0,10),"%u",set maxbits for expert mode);
    HANDLE_ARG(maxprec,(uint) strtol(argv[i]+len2,0,10),"%u",set maxprec for expert mode);
    HANDLE_ARG(minexp,(int) strtol(argv[i]+len2,0,10),"%d",set minexp for expert mode);

    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);

    /* Put this after setup_filter to permit printing of otherwise hard to 
       construct cd_values to facilitate manual invokation of h5repack */
    HANDLE_ARG(help,(int)strtol(argv[i]+len2,0,10),"%d",this help message); /* must be last for help to work */

    gen_data(DSIZE, 0.01, 10, (void**)&buf, TYPDBL);

    H5Eset_auto(H5E_DEFAULT, 0, 0);

    /* setup the 1D data space */
    if (0 > (sid = H5Screate_simple(1, chunk, 0))) ERROR(H5Screate_simple);

    /* create HDF5 file */
    if (0 > (fid = H5Fcreate(FNAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))) ERROR(H5Fcreate);

    /* test incorrect data type */
    tid = H5Tcreate(H5T_STRING, 8);
    if (0 <= (dsid = H5Dcreate(fid, "bad_type", tid, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) ERROR(H5Dcreate);
    assert(check_hdf5_error_stack_for_string("requires datatype class of H5T_FLOAT or H5T_INTEGER"));
    H5Tclose(tid);

    /* test invalid size of data type */
    tid = H5Tcopy(H5T_NATIVE_DOUBLE);
    H5Tset_size(tid, 9);
    if (0 <= (dsid = H5Dcreate(fid, "bad_type_size", tid, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) ERROR(H5Dcreate);
    assert(check_hdf5_error_stack_for_string("requires datatype size of 4 or 8"));
    H5Tclose(tid);

    /* test invalid chunking on highd data */
    cpid = setup_filter(5, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 <= (dsid = H5Dcreate(fid, "bad_chunking", H5T_NATIVE_FLOAT, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) ERROR(H5Dcreate);
    assert(check_hdf5_error_stack_for_string("chunk must have only 1...4 non-unity dimensions"));
    H5Pclose(cpid);

    /* write a compressed dataset to be corrupted later */
    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 > (dsid = H5Dcreate(fid, "corrupted_data", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) ERROR(H5Dcreate);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) ERROR(H5Dwrite);
    off = 3496; // H5Dget_offset(dsid);
    siz = H5Dget_storage_size(dsid);
    if (0 > H5Dclose(dsid)) ERROR(H5Dclose);
    if (0 > H5Pclose(cpid)) ERROR(H5Pclose);

    /* write a compressed dataset with some nans and infs */
    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 > (dsid = H5Dcreate(fid, "nans_and_infs", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) ERROR(H5Dcreate);
    memcpy(rbuf, buf, sizeof(rbuf));
    for (i = 7; i < 7+4; i++)
        rbuf[i] = d/(d-1.0);
    rbuf[42] = sqrt((double)-1.0);
    rbuf[42+1] = sqrt((double)-1.0);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) ERROR(H5Dwrite);
    if (0 > H5Dclose(dsid)) ERROR(H5Dclose);
    if (0 > H5Pclose(cpid)) ERROR(H5Pclose);
    if (0 > H5Fclose(fid)) ERROR(H5Fclose);

    /* Use raw file I/O to corrupt the dataset named corrupted_data */
    fd = open(FNAME, O_RDWR);
    pwrite(fd, corrupt, sizeof(corrupt), (off_t) off + (off_t) siz / 3);
    close(fd);

    /* Now, open the file with the nans_and_infs and corrupted datasets and try to read them */
    if (0 > (fid = H5Fopen(FNAME, H5F_ACC_RDONLY, H5P_DEFAULT))) ERROR(H5Fopen);
    if (0 > (dsid = H5Dopen(fid, "nans_and_infs", H5P_DEFAULT))) ERROR(H5Dopen);
    if (0 > H5Dread(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) ERROR(H5Dread);
    if (0 > H5Dclose(dsid)) ERROR(H5Dclose);
    for (i = 0, ndiffs = 0; i < DSIZE; i++)
    {
        double d = fabs(rbuf[i] - buf[i]);
        if (d > acc) ndiffs++;
    }
    assert(ndiffs == 10);
    if (0 > (dsid = H5Dopen(fid, "corrupted_data", H5P_DEFAULT))) ERROR(H5Dopen);
    if (0 > H5Dread(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) ERROR(H5Dread);
    for (i = 0, ndiffs = 0; i < DSIZE; i++)
    {
        double d = fabs(rbuf[i] - buf[i]);
        if (d > acc) ndiffs++;
    }
    assert(ndiffs == 1408);
    if (0 > H5Dclose(dsid)) ERROR(H5Dclose);

    free(buf);
    if (0 > H5Fclose(fid)) ERROR(H5Fclose);
    H5close();

    return 0;
}
