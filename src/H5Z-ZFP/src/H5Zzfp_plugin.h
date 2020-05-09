#ifndef H5Z_ZFP_PLUGIN_H
#define H5Z_ZFP_PLUGIN_H

/* Filter ID number registered with The HDF Group */
#define H5Z_FILTER_ZFP 32013

#define H5Z_FILTER_ZFP_VERSION_MAJOR 1
#define H5Z_FILTER_ZFP_VERSION_MINOR 0
#define H5Z_FILTER_ZFP_VERSION_PATCH 1

#define H5Z_ZFP_MODE_RATE      1
#define H5Z_ZFP_MODE_PRECISION 2
#define H5Z_ZFP_MODE_ACCURACY  3
#define H5Z_ZFP_MODE_EXPERT    4
#define H5Z_ZFP_MODE_REVERSIBLE 5

#define H5Z_ZFP_CD_NELMTS_MEM ((size_t) 6) /* used in public API to filter */
#define H5Z_ZFP_CD_NELMTS_MAX ((size_t) 6) /* max, over all versions, used in dataset header */

/* HDF5 filter cd_vals[] layout (6 unsigned ints)
cd_vals    0       1        2         3         4         5    
----------------------------------------------------------------
rate:      1    unused    rateA     rateB     unused    unused
precision: 2    unused    prec      unused    unused    unused
accuracy:  3    unused    accA      accB      unused    unused
expert:    4    unused    minbits   maxbits   maxprec   minexp

A/B are high/low words of a double.

Note: This is *NOT* the same layout that is ultimately stored
to the file. A wholly different, cd_vals is stored in the file
using zfp_write_header.
*/

#define H5Pset_zfp_rate_cdata(R, N, CD)          \
do { if (N>=4) {double *p = (double *) &CD[2];   \
CD[0]=CD[1]=CD[2]=CD[3]=0;                       \
CD[0]=H5Z_ZFP_MODE_RATE; *p=R; N=4;}} while(0)

#define H5Pget_zfp_rate_cdata(N, CD) \
((double)(((N>=4)&&(CD[0]==H5Z_ZFP_MODE_RATE))?*((double *) &CD[2]):-1))

#define H5Pset_zfp_precision_cdata(P, N, CD)  \
do { if (N>=3) {CD[0]=CD[1]=CD[2];            \
CD[0]=H5Z_ZFP_MODE_PRECISION;                 \
CD[2]=P; N=3;}} while(0)

#define H5Pget_zfp_precision_cdata(N, CD) \
((double)(((N>=3)&&(CD[0]==H5Z_ZFP_MODE_ACCURACY))?CD[2]:-1))

#define H5Pset_zfp_accuracy_cdata(A, N, CD)      \
do { if (N>=4) {double *p = (double *) &CD[2];   \
CD[0]=CD[1]=CD[2]=CD[3]=0;                       \
CD[0]=H5Z_ZFP_MODE_ACCURACY; *p=A; N=4;}} while(0)

#define H5Pget_zfp_accuracy_cdata(N, CD) \
((double)(((N>=4)&&(CD[0]==H5Z_ZFP_MODE_ACCURACY))?*((double *) &CD[2]):-1))

#define H5Pset_zfp_expert_cdata(MiB, MaB, MaP, MiE, N, CD) \
do { if (N>=6) { CD[0]=CD[1]=CD[2]=CD[3]=CD[4]=CD[5]=0;    \
CD[0]=H5Z_ZFP_MODE_EXPERT;                                 \
CD[2]=MiB; CD[3]=MaB; CD[4]=MaP;                           \
CD[5]=(unsigned int)MiE; N=6;}} while(0)

#define H5Pget_zfp_expert_cdata(N, CD, MiB, MaB, MaP, MiE) \
do {                                                    \
    if ((N>=6)&&(CD[0] == H5Z_ZFP_MODE_EXPERT))         \
    {                                                   \
        unsigned int *p; int *q;                        \
        p = MiB; *p = CD[2];                            \
        p = MaB; *p = CD[3];                            \
        p = MaP; *p = CD[4];                            \
        q = MiE; *q = (int) CD[5];                      \
    }                                                   \
} while(0)

#define H5Pset_zfp_reversible_cdata(N, CD)       \
do { if (N>=1) {                                 \
CD[0]=H5Z_ZFP_MODE_REVERSIBLE; N=1;}} while(0)

#define H5Pget_zfp_reversible_cdata(N, CD) \
((int)(((N>=1)&&(CD[0]==H5Z_ZFP_MODE_REVERSIBLE))?1:-1))

#endif
