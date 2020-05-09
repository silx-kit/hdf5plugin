#ifndef H5Z_ZFP_PROPS_H
#define H5Z_ZFP_PROPS_H

#include "hdf5.h"

#ifdef __cplusplus
extern "C" {
#endif

extern herr_t H5Pset_zfp_rate(hid_t plist, double rate); 
extern herr_t H5Pset_zfp_precision(hid_t plist, unsigned int prec); 
extern herr_t H5Pset_zfp_accuracy(hid_t plist, double acc); 
extern herr_t H5Pset_zfp_expert(hid_t plist, unsigned int minbits, unsigned int maxbits,
    unsigned int maxprec, int minexp); 
extern herr_t H5Pset_zfp_reversible(hid_t plist); 

#ifdef __cplusplus
}
#endif

#endif
