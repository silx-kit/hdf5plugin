/*
Copyright (c) 2016, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Mark C. Miller, miller86@llnl.gov
LLNL-CODE-707197. All rights reserved.

This file is part of H5Z-ZFP. Please also read the BSD license
https://raw.githubusercontent.com/LLNL/H5Z-ZFP/master/LICENSE
*/

#include "test_common.h"

#ifdef H5Z_ZFP_USE_PLUGIN
#include "H5Zzfp_plugin.h"
#else
#include "H5Zzfp_lib.h"
#include "H5Zzfp_props.h"
#endif

static hid_t setup_filter(int n, hsize_t *chunk, int zfpmode,
    double rate, double acc, unsigned int prec,
    unsigned int minbits, unsigned int maxbits, unsigned int maxprec, int minexp)
{
    hid_t cpid;

    /* setup dataset creation properties */
    if (0 > (cpid = H5Pcreate(H5P_DATASET_CREATE))) SET_ERROR(H5Pcreate);
    if (0 > H5Pset_chunk(cpid, n, chunk)) SET_ERROR(H5Pset_chunk);

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
    int i, ndiffs;
    unsigned corrupt[4] = {0xDeadBeef,0xBabeFace, 0xDeadBabe, 0xBeefFace};
    double d = 1.0, *buf = 0, rbuf[DSIZE];
    hsize_t chunk[] = {DSIZE,16,16,16,16};
    haddr_t off;
    hsize_t siz;

    /* HDF5 related variables */
    hid_t fid, tid, dsid, sid, cpid;

    int help = 0;

    /* compression parameters (defaults taken from ZFP header) */
    int zfpmode = H5Z_ZFP_MODE_ACCURACY;
    double rate = 4;
    double acc = 0.1;
    unsigned int prec = 11;
    unsigned int minbits = 0;
    unsigned int maxbits = 4171;
    unsigned int maxprec = 64;
    int minexp = -1074;

    /* ZFP filter arguments */
    HANDLE_SEP(ZFP compression parameters)
    HANDLE_ARG(zfpmode,(int) strtol(argv[i]+len2,0,10),"%d", (1=rate,2=prec,3=acc,4=expert,5=reversible));
    HANDLE_ARG(rate,(double) strtod(argv[i]+len2,0),"%g",set rate for rate mode);
    HANDLE_ARG(acc,(double) strtod(argv[i]+len2,0),"%g",set accuracy for accuracy mode);
    HANDLE_ARG(prec,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set precision for precision mode);
    HANDLE_ARG(minbits,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set minbits for expert mode);
    HANDLE_ARG(maxbits,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set maxbits for expert mode);
    HANDLE_ARG(maxprec,(unsigned int) strtol(argv[i]+len2,0,10),"%u",set maxprec for expert mode);
    HANDLE_ARG(minexp,(int) strtol(argv[i]+len2,0,10),"%d",set minexp for expert mode);

    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);

    /* Put this after setup_filter to permit printing of otherwise hard to
       construct cd_values to facilitate manual invokation of h5repack */
    HANDLE_ARG(help,(int)strtol(argv[i]+len2,0,10),"%d",this help message); /* must be last for help to work */

    gen_data(DSIZE, 0.01, 10, (void**)&buf, TYPDBL);

    H5Eset_auto(H5E_DEFAULT, 0, 0);

    /* setup the 1D data space */
    if (0 > (sid = H5Screate_simple(1, chunk, 0))) SET_ERROR(H5Screate_simple);

    /* create HDF5 file */
    if (0 > (fid = H5Fcreate(FNAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))) SET_ERROR(H5Fcreate);

    /* test incorrect data type */
    tid = H5Tcreate(H5T_STRING, 8);
    if (0 <= (dsid = H5Dcreate(fid, "bad_type", tid, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION<=0x052
    assert(check_hdf5_error_stack_for_string("requires datatype class of H5T_FLOAT"));
#else
    assert(check_hdf5_error_stack_for_string("requires datatype class of H5T_FLOAT or H5T_INTEGER"));
#endif
    H5Tclose(tid);

    /* test invalid size of data type */
    tid = H5Tcopy(H5T_NATIVE_DOUBLE);
    H5Tset_size(tid, 9);
    if (0 <= (dsid = H5Dcreate(fid, "bad_type_size", tid, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
    assert(check_hdf5_error_stack_for_string("requires datatype size of 4 or 8"));
    H5Tclose(tid);

    /* test invalid chunking on highd data */
    cpid = setup_filter(5, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 <= (dsid = H5Dcreate(fid, "bad_chunking", H5T_NATIVE_FLOAT, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
#if defined(ZFP_LIB_VERSION) && ZFP_LIB_VERSION<=0x053
    assert(check_hdf5_error_stack_for_string("chunk must have only 1...3 non-unity dimensions"));
#else
    assert(check_hdf5_error_stack_for_string("chunk must have only 1...4 non-unity dimensions"));
#endif
    H5Pclose(cpid);

    /* write a compressed dataset to be corrupted later */
    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 > (dsid = H5Dcreate(fid, "corrupted_data", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf)) SET_ERROR(H5Dwrite);
    off = 3496; // H5Dget_offset(dsid);
    siz = H5Dget_storage_size(dsid);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
    if (0 > H5Pclose(cpid)) SET_ERROR(H5Pclose);

    /* write a compressed dataset with some nans and infs */
    cpid = setup_filter(1, chunk, zfpmode, rate, acc, prec, minbits, maxbits, maxprec, minexp);
    if (0 > (dsid = H5Dcreate(fid, "nans_and_infs", H5T_NATIVE_DOUBLE, sid, H5P_DEFAULT, cpid, H5P_DEFAULT))) SET_ERROR(H5Dcreate);
    memcpy(rbuf, buf, sizeof(rbuf));
    for (i = 7; i < 7+4; i++)
        rbuf[i] = d/(d-1.0);
    rbuf[42] = sqrt((double)-1.0);
    rbuf[42+1] = sqrt((double)-1.0);
    if (0 > H5Dwrite(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) SET_ERROR(H5Dwrite);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
    if (0 > H5Pclose(cpid)) SET_ERROR(H5Pclose);
    if (0 > H5Fclose(fid)) SET_ERROR(H5Fclose);

    /* Use raw file I/O to corrupt the dataset named corrupted_data */
    FILE *fp;
    fp = fopen(FNAME, "rb+");
    if(fp == NULL) SET_ERROR(fopen);
    fseek(fp, (off_t) off + (off_t) siz / 3, SEEK_SET);
    fwrite(corrupt, 1 , sizeof(corrupt), fp);
    fclose(fp);

    /* Now, open the file with the nans_and_infs and corrupted datasets and try to read them */
    if (0 > (fid = H5Fopen(FNAME, H5F_ACC_RDONLY, H5P_DEFAULT))) SET_ERROR(H5Fopen);
    if (0 > (dsid = H5Dopen(fid, "nans_and_infs", H5P_DEFAULT))) SET_ERROR(H5Dopen);
    if (0 > H5Dread(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) SET_ERROR(H5Dread);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);
    for (i = 0, ndiffs = 0; i < DSIZE; i++)
    {
        double d = fabs(rbuf[i] - buf[i]);
        if (d > acc) ndiffs++;
    }
    assert(ndiffs == 10);
    if (0 > (dsid = H5Dopen(fid, "corrupted_data", H5P_DEFAULT))) SET_ERROR(H5Dopen);
    if (0 > H5Dread(dsid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf)) SET_ERROR(H5Dread);
    for (i = 0, ndiffs = 0; i < DSIZE; i++)
    {
        double d = fabs(rbuf[i] - buf[i]);
        if (d > acc) ndiffs++;
    }
    assert(ndiffs == 1408);
    if (0 > H5Dclose(dsid)) SET_ERROR(H5Dclose);

    free(buf);
    if (0 > H5Fclose(fid)) SET_ERROR(H5Fclose);
    H5close();

    return 0;
}
