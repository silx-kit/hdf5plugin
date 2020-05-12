MODULE H5Zzfp_props_f

  USE ISO_C_BINDING
  USE HDF5
  IMPLICIT NONE

  INTEGER :: H5Z_FILTER_ZFP=32013

  INTEGER :: H5Z_FILTER_ZFP_VERSION_MAJOR=1
  INTEGER :: H5Z_FILTER_ZFP_VERSION_MINOR=0
  INTEGER :: H5Z_FILTER_ZFP_VERSION_PATCH=1
  
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MEM=6  ! used in public API to filter
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MAX=6  ! max, over all versions, used in dataset header

  INTEGER, PARAMETER :: H5Z_ZFP_MODE_RATE       = 1
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_PRECISION  = 2
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_ACCURACY   = 3
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_EXPERT     = 4
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_REVERSIBLE = 5

  INTERFACE
     INTEGER(C_INT) FUNCTION H5Z_zfp_initialize() BIND(C, NAME='H5Z_zfp_initialize')
       IMPORT :: C_INT
       IMPLICIT NONE
     END FUNCTION H5Z_zfp_initialize

     INTEGER(C_INT) FUNCTION H5Z_zfp_finalize() BIND(C, NAME='H5Z_zfp_finalize')
       IMPORT :: C_INT
       IMPLICIT NONE
     END FUNCTION H5Z_zfp_finalize

     INTEGER(C_INT) FUNCTION H5Pset_zfp_rate(plist, rate) BIND(C, NAME='H5Pset_zfp_rate')
       IMPORT :: C_INT, C_DOUBLE, HID_T
       IMPLICIT NONE
       INTEGER(HID_T), VALUE :: plist
       REAL(C_DOUBLE), VALUE :: rate
     END FUNCTION H5Pset_zfp_rate

     INTEGER(C_INT) FUNCTION H5Pset_zfp_precision(plist, prec) BIND(C, NAME='H5Pset_zfp_precision')
       IMPORT :: C_INT, HID_T
       IMPLICIT NONE
       INTEGER(HID_T), VALUE :: plist
       INTEGER(C_INT), VALUE :: prec
     END FUNCTION H5Pset_zfp_precision
     
     INTEGER(C_INT) FUNCTION H5Pset_zfp_accuracy(plist, acc) BIND(C, NAME='H5Pset_zfp_accuracy')
       IMPORT :: C_INT, C_DOUBLE, HID_T
       IMPLICIT NONE
       INTEGER(HID_T), VALUE :: plist
       REAL(C_DOUBLE), VALUE :: acc
     END FUNCTION H5Pset_zfp_accuracy

     INTEGER(C_INT) FUNCTION H5Pset_zfp_expert(plist, minbits, maxbits, maxprec, minexp) &
          BIND(C, NAME='H5Pset_zfp_expert')
       IMPORT :: C_INT, HID_T
       IMPLICIT NONE
       INTEGER(HID_T), VALUE :: plist
       INTEGER(C_INT), VALUE :: minbits
       INTEGER(C_INT), VALUE :: maxbits
       INTEGER(C_INT), VALUE :: maxprec
       INTEGER(C_INT), VALUE :: minexp
     END FUNCTION H5Pset_zfp_expert

     INTEGER(C_INT) FUNCTION H5Pset_zfp_reversible(plist) BIND(C, NAME='H5Pset_zfp_reversible')
       IMPORT :: C_INT, HID_T
       IMPLICIT NONE
       INTEGER(HID_T), VALUE :: plist
     END FUNCTION H5Pset_zfp_reversible
    
  END INTERFACE

END MODULE H5Zzfp_props_f
