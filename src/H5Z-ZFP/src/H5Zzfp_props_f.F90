MODULE H5Zzfp_props_f

#include "H5Zzfp_version.h"

  USE ISO_C_BINDING
  USE HDF5
  IMPLICIT NONE

  ! First, create _F equivalent parameters for all the C interface's CPP symbols and
  ! capture the original C interface's CPP values.
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_F=H5Z_FILTER_ZFP

  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_MAJOR_F=H5Z_FILTER_ZFP_VERSION_MAJOR
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_MINOR_F=H5Z_FILTER_ZFP_VERSION_MINOR
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_PATCH_F=H5Z_FILTER_ZFP_VERSION_PATCH
  
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MEM_F=H5Z_ZFP_CD_NELMTS_MEM
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MAX_F=H5Z_ZFP_CD_NELMTS_MAX

  INTEGER, PARAMETER :: H5Z_ZFP_MODE_RATE_F       = H5Z_ZFP_MODE_RATE
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_PRECISION_F  = H5Z_ZFP_MODE_PRECISION
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_ACCURACY_F   = H5Z_ZFP_MODE_ACCURACY
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_EXPERT_F     = H5Z_ZFP_MODE_EXPERT
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_REVERSIBLE_F = H5Z_ZFP_MODE_REVERSIBLE

  ! Next, undef all the C interface's CPP symbols so we can declare Fortran parameters
  ! with the identical names
#undef H5Z_FILTER_ZFP
#undef H5Z_FILTER_ZFP_VERSION_MAJOR
#undef H5Z_FILTER_ZFP_VERSION_MINOR
#undef H5Z_FILTER_ZFP_VERSION_PATCH
#undef H5Z_ZFP_CD_NELMTS_MEM
#undef H5Z_ZFP_CD_NELMTS_MAX
#undef H5Z_ZFP_MODE_RATE
#undef H5Z_ZFP_MODE_PRECISION
#undef H5Z_ZFP_MODE_ACCURACY
#undef H5Z_ZFP_MODE_EXPERT
#undef H5Z_ZFP_MODE_REVERSIBLE

  ! Define Fortran parameters using the original _F values captured above.
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP=H5Z_FILTER_ZFP_F

  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_MAJOR=H5Z_FILTER_ZFP_VERSION_MAJOR_F
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_MINOR=H5Z_FILTER_ZFP_VERSION_MINOR_F
  INTEGER, PARAMETER :: H5Z_FILTER_ZFP_VERSION_PATCH=H5Z_FILTER_ZFP_VERSION_PATCH_F
  
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MEM=H5Z_ZFP_CD_NELMTS_MEM_F
  INTEGER(C_SIZE_T), PARAMETER :: H5Z_ZFP_CD_NELMTS_MAX=H5Z_ZFP_CD_NELMTS_MAX_F

  INTEGER, PARAMETER :: H5Z_ZFP_MODE_RATE       = H5Z_ZFP_MODE_RATE_F
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_PRECISION  = H5Z_ZFP_MODE_PRECISION_F
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_ACCURACY   = H5Z_ZFP_MODE_ACCURACY_F
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_EXPERT     = H5Z_ZFP_MODE_EXPERT_F
  INTEGER, PARAMETER :: H5Z_ZFP_MODE_REVERSIBLE = H5Z_ZFP_MODE_REVERSIBLE_F

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

     SUBROUTINE H5Pset_zfp_rate_cdata(rate, cd_nelmts, cd_values) BIND(C, NAME='H5Pset_zfp_rate_cdata_f')
       IMPORT :: C_DOUBLE, C_INT, C_SIZE_T
       IMPLICIT NONE
       REAL(C_DOUBLE), VALUE :: rate
       INTEGER(C_SIZE_T) :: cd_nelmts
       INTEGER(C_INT), DIMENSION(*) :: cd_values
     END SUBROUTINE H5Pset_zfp_rate_cdata

     SUBROUTINE H5Pset_zfp_accuracy_cdata(acc, cd_nelmts, cd_values) BIND(C, NAME='H5Pset_zfp_accuracy_cdata_f')
       IMPORT :: C_DOUBLE, C_INT, C_SIZE_T
       IMPLICIT NONE
       REAL(C_DOUBLE), VALUE :: acc
       INTEGER(C_SIZE_T) :: cd_nelmts
       INTEGER(C_INT), DIMENSION(*) :: cd_values
     END SUBROUTINE H5Pset_zfp_accuracy_cdata

    SUBROUTINE H5Pset_zfp_precision_cdata(prec, cd_nelmts, cd_values) BIND(C, NAME='H5Pset_zfp_precision_cdata_f')
       IMPORT :: C_INT, C_SIZE_T
       IMPLICIT NONE
       INTEGER(C_INT), VALUE :: prec
       INTEGER(C_SIZE_T) :: cd_nelmts
       INTEGER(C_INT), DIMENSION(*) :: cd_values
     END SUBROUTINE H5Pset_zfp_precision_cdata

    SUBROUTINE H5Pset_zfp_expert_cdata(minbits, maxbits, maxprec, minexp, cd_nelmts, cd_values) &
         BIND(C, NAME='H5Pset_zfp_expert_cdata_f')
       IMPORT :: C_INT, C_SIZE_T
       IMPLICIT NONE
       INTEGER(C_INT), VALUE :: minbits, maxbits, maxprec, minexp
       INTEGER(C_SIZE_T) :: cd_nelmts
       INTEGER(C_INT), DIMENSION(*) :: cd_values
     END SUBROUTINE H5Pset_zfp_expert_cdata

   SUBROUTINE H5Pset_zfp_reversible_cdata(cd_nelmts, cd_values) BIND(C, NAME='H5Pset_zfp_reversible_cdata_f')
       IMPORT :: C_INT, C_SIZE_T
       IMPLICIT NONE
       INTEGER(C_SIZE_T) :: cd_nelmts
       INTEGER(C_INT), DIMENSION(*) :: cd_values
     END SUBROUTINE H5Pset_zfp_reversible_cdata

  END INTERFACE

END MODULE H5Zzfp_props_f
