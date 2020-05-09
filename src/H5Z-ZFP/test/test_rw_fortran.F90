PROGRAM main

  USE ISO_C_BINDING
  USE HDF5
  USE h5zzfp_props_f
  IMPLICIT NONE

  INTEGER, PARAMETER :: dp = C_DOUBLE

  INTEGER, PARAMETER :: NAME_LEN=256
  INTEGER, PARAMETER :: DIM0=32
  INTEGER, PARAMETER :: DIM1=64
  INTEGER, PARAMETER :: CHUNK0=4
  INTEGER, PARAMETER :: CHUNK1=8

  INTEGER :: i
  INTEGER(hsize_t) :: j

  ! sinusoid data generation variables 
  INTEGER(hsize_t) :: npoints

  ! compression parameters (defaults taken from ZFP header)
  integer(C_INT) :: zfpmode = 3 !1=rate, 2=prec, 3=acc, 4=expert
  REAL(dp) :: rate = 4_c_double
  REAL(dp) :: acc = 0_c_double
  integer(C_INT) :: prec = 11
  integer(C_INT), PARAMETER :: minbits = 0
  integer(C_INT), PARAMETER :: maxbits = 4171
  integer(C_INT), PARAMETER :: maxprec = 64
  integer(C_INT), PARAMETER :: minexp = -1074
  
  ! HDF5 related variables
  INTEGER(hid_t) fid, dsid, sid, cpid, dcpl_id, space_id
  INTEGER(C_INT), DIMENSION(1:H5Z_ZFP_CD_NELMTS_MEM) :: cd_values
  INTEGER(C_SIZE_T) :: cd_nelmts = H5Z_ZFP_CD_NELMTS_MEM

  ! compressed/uncompressed difference stat variables 
  REAL(dp) :: max_absdiff = 0
  REAL(dp) :: max_reldiff = 0
  INTEGER(C_INT) :: num_diffs = 0

  REAL(dp) :: noise = 0.001
  REAL(dp) :: amp = 17.7

  REAL(dp), DIMENSION(1:DIM0,1:DIM1), TARGET :: wdata
  INTEGER(hsize_t), DIMENSION(1:2) ::  dims = (/DIM0, DIM1/)
  INTEGER(hsize_t), DIMENSION(1:2) ::  chunk2 = (/CHUNK0, CHUNK1/)
  REAL(dp), DIMENSION(:), ALLOCATABLE, TARGET :: obuf, cbuf
  CHARACTER(LEN=180) :: ofile="test_zfp_fortran.h5"

  INTEGER :: status
  TYPE(C_PTR) :: f_ptr
  INTEGER*4 H5Z_FLAG_MANDATORY/Z'00000000'/
  REAL(dp) :: absdiff, reldiff

  INTERFACE
     LOGICAL FUNCTION real_eq(a,b,ulp)
       USE ISO_C_BINDING
       IMPLICIT NONE
       REAL(C_DOUBLE), INTENT (in):: a,b
       REAL(C_DOUBLE) :: Rel
       INTEGER, OPTIONAL, INTENT( IN )  :: ulp
     END FUNCTION real_eq
  END INTERFACE

  CHARACTER(LEN=10)  :: arg
  INTEGER :: len
  LOGICAL :: write_only = .FALSE., avail
  INTEGER     :: config_flag = 0   ! for h5zget_filter_info_f
  INTEGER     :: config_flag_both = 0   ! for h5zget_filter_info_f
  INTEGER :: nerr = 0
  
  DO i = 1, COMMAND_ARGUMENT_COUNT()
     CALL GET_COMMAND_ARGUMENT(i,arg,len,status)
     IF (status .NE. 0) THEN
        WRITE (*,*) 'get_command_argument failed: status = ', status, ' arg = ', i
        STOP
     END IF
     IF(arg(1:len).EQ.'zfpmode') THEN
        CALL GET_COMMAND_ARGUMENT(i+1,arg,len,status)
        IF (status .NE. 0) THEN
           WRITE (*,*) 'get_command_argument failed: status = ', status, ' arg = ', i
           STOP
        END IF
        READ(arg(1:len), *) zfpmode
     ELSE IF (arg(1:len).EQ.'rate')THEN
        CALL GET_COMMAND_ARGUMENT(i+1,arg,len,status)
        IF (status .NE. 0) THEN
           WRITE (*,*) 'get_command_argument failed: status = ', status, ' arg = ', i
           STOP
        END IF
        READ(arg(1:len), *) rate
     ELSE IF (arg(1:len).EQ.'acc')THEN
        CALL GET_COMMAND_ARGUMENT(i+1,arg,len,status)
        IF (status .NE. 0) THEN
           WRITE (*,*) 'get_command_argument failed: status = ', status, ' arg = ', i
           STOP
        END IF
        READ(arg(1:len), *) acc
     ELSE IF (arg(1:len).EQ.'prec')THEN
        CALL GET_COMMAND_ARGUMENT(i+1,arg,len,status)
        IF (status .NE. 0) THEN
           WRITE (*,*) 'get_command_argument failed: status = ', status, ' arg = ', i
           STOP
        END IF
        READ(arg(1:len), *) prec
     ELSE IF (arg(1:len).EQ.'write')THEN
        write_only = .TRUE.

     ELSE IF (INDEX(arg(1:len),'help').NE.0)THEN
        PRINT*," *** USAGE *** "
        PRINT*,"zfpmode <val> - 1=rate,2=prec,3=acc,4=expert,5=reversible"
        PRINT*,"rate <val>    - set rate for rate mode of filter"
        PRINT*,"acc <val>     - set accuracy for accuracy mode of filter"
        PRINT*,"prec <val>    - set PRECISION for PRECISION mode of zfp filter"
        PRINT*,"write         - only write the file"
        STOP
     ENDIF
      
  END DO

  ! create data to write if we're not reading from an existing file 
 
  CALL gen_data(INT(dim1*dim0, c_size_t), noise, amp, wdata)

  CALL h5open_f(status)
  CALL check("h5open_f", status, nerr)

  ! initialize the ZFP filter
  status = H5Z_zfp_initialize()
  CALL check("H5Z_zfp_initialize", status, nerr)

  ! create HDF5 file 
  CALL h5fcreate_f(ofile, H5F_ACC_TRUNC_F, fid, status)
  CALL check("h5fcreate_f", status, nerr)
  
  ! setup dataset compression via cd_values

  CALL h5pcreate_f(H5P_DATASET_CREATE_F, cpid, status)
  CALL check("h5pcreate_f", status, nerr)
  CALL h5pset_chunk_f(cpid, 2, chunk2, status)
  CALL check("h5pset_chunk_f", status, nerr)
  
  cd_nelmts = 4
  cd_values(1:cd_nelmts) = (/3,0,0,0/) 

  CALL H5Pset_filter_f(cpid, H5Z_FILTER_ZFP, H5Z_FLAG_MANDATORY, cd_nelmts, cd_values, status)
  CALL check("H5Pset_filter_f", status, nerr)
  
  !
  ! Check that filter is registered with the library now.
  ! If it is registered, retrieve filter's configuration.
  !
  CALL H5Zfilter_avail_f(H5Z_FILTER_ZFP, avail, status)
  CALL check("H5Zfilter_avail_f", status, nerr)

  IF (avail) THEN
     CALL h5zget_filter_info_f(H5Z_FILTER_ZFP, config_flag, status)
     CALL check("h5zget_filter_info_f", status, nerr)
     !
     ! Make sure h5zget_filter_info_f returns the right flag
     !
     config_flag_both=IOR(H5Z_FILTER_ENCODE_ENABLED_F,H5Z_FILTER_DECODE_ENABLED_F)
     IF (config_flag .NE. config_flag_both) THEN
        IF(config_flag .NE. H5Z_FILTER_DECODE_ENABLED_F)  THEN
           PRINT*,'h5zget_filter_info_f config_flag failed'
        ENDIF
     ENDIF
  ENDIF

  ! setup the 2D data space
  CALL h5screate_simple_f(2, dims, sid, status)
  CALL check("h5screate_simple_f", status, nerr)

  ! write the data WITHOUT compression 
  CALL h5dcreate_f(fid, "original", H5T_NATIVE_DOUBLE, sid, dsid, status)
  CALL check("h5dcreate_f", status, nerr)
  f_ptr = C_LOC(wdata(1,1))
  CALL h5dwrite_f(dsid, H5T_NATIVE_DOUBLE, f_ptr, status)
  CALL check("h5dwrite_f", status, nerr)
  CALL h5dclose_f(dsid,status)
  CALL check("h5dclose_f", status, nerr)

  ! write data using default parameters
  CALL h5dcreate_f(fid, "compressed_default", H5T_NATIVE_DOUBLE, sid, dsid, status, dcpl_id=cpid)
  CALL check("h5dcreate_f", status, nerr)
  f_ptr = C_LOC(wdata(1,1))
  CALL h5dwrite_f(dsid, H5T_NATIVE_DOUBLE, f_ptr, status)
  CALL check("h5dwrite_f", status, nerr)
  IF(status.NE.0) PRINT*,"h5dwrite_f failed"
  CALL h5dclose_f(dsid,status)
  CALL check("h5dclose_f", status, nerr)

  ! write the data using properties
  CALL H5Premove_filter_f(cpid, H5Z_FILTER_ZFP, status)
  IF (zfpmode .EQ. H5Z_ZFP_MODE_RATE) THEN
     status = H5Pset_zfp_rate(cpid, rate)
     CALL check("H5Pset_zfp_rate", status, nerr)
  ELSE IF (zfpmode .EQ. H5Z_ZFP_MODE_PRECISION) THEN
     status = H5Pset_zfp_precision(cpid, prec)
     CALL check("H5Pset_zfp_precision", status, nerr)
  ELSE IF (zfpmode .EQ. H5Z_ZFP_MODE_ACCURACY)THEN
     status = H5Pset_zfp_accuracy(cpid, acc)
     CALL check("H5Pset_zfp_accuracy", status, nerr)
  ELSE IF (zfpmode .EQ. H5Z_ZFP_MODE_EXPERT) THEN
     status = H5Pset_zfp_expert(cpid, minbits, maxbits, maxprec, minexp)
     CALL check("H5Pset_zfp_expert", status, nerr)
  ELSE IF (zfpmode .EQ. H5Z_ZFP_MODE_REVERSIBLE) THEN
     status = H5Pset_zfp_reversible(cpid)
     CALL check("H5Pset_zfp_reversible", status, nerr)
  ENDIF
  CALL h5dcreate_f(fid, "compressed", H5T_NATIVE_DOUBLE, sid, dsid, status, dcpl_id=cpid)
  CALL check("h5dcreate_f", status, nerr)
  f_ptr = C_LOC(wdata(1,1))
  CALL h5dwrite_f(dsid, H5T_NATIVE_DOUBLE, f_ptr, status)
  CALL check("h5dwrite_f", status, nerr)
  CALL h5dclose_f(dsid,status)
  CALL check("h5dclose_f", status, nerr)

  ! clean up
  CALL h5pclose_f(cpid, status)
  CALL check("", status, nerr)
  CALL h5sclose_f(sid, status)
  CALL check("", status, nerr)
  CALL h5fclose_f(fid, status)
  CALL check("", status, nerr)

  IF(write_only) STOP

  CALL h5fopen_f(ofile, H5F_ACC_RDONLY_F, fid, status)
  CALL check("h5fopen_f", status, nerr)
  
  ! read the original dataset 
  CALL h5dopen_f (fid, "original", dsid, status)
  CALL check("h5dopen_f", status, nerr)

  CALL h5dget_space_f(dsid, space_id,status) 
  CALL check("h5dget_space_f", status, nerr)
  CALL H5Sget_simple_extent_npoints_f(space_id, npoints, status)
  CALL check("H5Sget_simple_extent_npoints_f", status, nerr)
  CALL H5Sclose_f(space_id, status)
  CALL check("H5Sclose_f", status, nerr)
  ALLOCATE(obuf(1:npoints))
  f_ptr = C_LOC(obuf(1))
  CALL H5Dread_f(dsid, H5T_NATIVE_DOUBLE, f_ptr, status)
  CALL check("H5Dread_f", status, nerr)
  CALL H5Dclose_f(dsid, status)
  CALL check("H5Dclose_f", status, nerr)

  ! read the compressed dataset 
  CALL h5dopen_f (fid, "compressed_default", dsid, status)
  CALL check("", status, nerr)
  CALL H5Dget_create_plist_f(dsid, dcpl_id, status )
  CALL check("", status, nerr)
  ALLOCATE(cbuf(1:npoints))
  f_ptr = C_LOC(cbuf(1))
  CALL H5Dread_f(dsid, H5T_NATIVE_DOUBLE, f_ptr, status)
  CALL check("H5Dread_f", status, nerr)
  CALL H5Dclose_f(dsid, status)
  CALL check("H5Dclose_f", status, nerr)

  ! clean up 
  CALL H5Pclose_f(dcpl_id, status)
  CALL check("H5Pclose_f", status, nerr)
  CALL H5Fclose_f(fid, status)
  CALL check("H5Fclose_f", status, nerr)

  ! compare to generated data
  DO j = 1, npoints
     absdiff = obuf(j) - cbuf(j)
     if(absdiff < 0) absdiff = -absdiff
     IF(absdiff > 0) THEN
         reldiff = 0
         IF (obuf(j) .NE. 0) reldiff = absdiff / obuf(j)
         
         IF (absdiff > max_absdiff) max_absdiff = absdiff
         IF (reldiff > max_reldiff) max_reldiff = reldiff

         IF( .NOT.real_eq(obuf(j), cbuf(j), 125) ) THEN
            num_diffs = num_diffs + 1
         ENDIF
      ENDIF
   ENDDO
   
   IF(num_diffs.NE.0)THEN
      WRITE(*,'(A)') "Fortran read/write test Failed"
      WRITE(*,'(I0," values are different; max-absdiff = ",E15.8,", max-reldiff = ",E15.8)')  &
           num_diffs,max_absdiff, max_reldiff
   ELSE IF(nerr.NE.0)THEN
      WRITE(*,'(A)') "Fortran read/write test Failed"
   ELSE
      WRITE(*,'(A)') "Fortran read/write test Passed"
   ENDIF

   DEALLOCATE(obuf, cbuf)

   ! initialize the ZFP filter
   status = H5Z_zfp_finalize()
   CALL check("H5Z_zfp_finalize", status, nerr)

   CALL H5close_f(status)

END PROGRAM main

! Generate a simple, 1D sinusioidal data array with some noise
SUBROUTINE gen_data(npoints, noise, amp, buf)
  
  USE ISO_C_BINDING
  IMPLICIT NONE
  INTEGER(C_SIZE_T) :: npoints
  REAL(C_DOUBLE) :: noise
  REAL(C_DOUBLE) :: amp
  REAL(C_DOUBLE), DIMENSION(1:npoints) :: buf

  INTEGER(C_SIZE_T) :: i
  REAL(C_DOUBLE) :: x
  REAL(C_DOUBLE) :: pi
  REAL(C_DOUBLE) :: rand
  pi = 4.0_C_DOUBLE*ATAN(1.0_C_DOUBLE)

  DO i = 1, npoints
     rand = i
     CALL RANDOM_NUMBER(rand)
     x = 2_c_double * pi * REAL(i-1, C_DOUBLE) / REAL(npoints-1, C_DOUBLE)
     buf(i) = amp*( 1_C_DOUBLE + SIN(x)) + (rand - 0.5)*noise
  ENDDO

END SUBROUTINE gen_data

LOGICAL FUNCTION real_eq(a,b,ulp)
  USE ISO_C_BINDING
  IMPLICIT NONE
  REAL(C_DOUBLE), INTENT (in):: a,b
  REAL(C_DOUBLE) :: Rel = 1.0_C_DOUBLE
  INTEGER, OPTIONAL, INTENT( IN )  :: ulp
  IF ( PRESENT( ulp ) )  Rel = REAL( ABS(ulp), C_DOUBLE)
  real_eq = ABS( a - b ) < ( Rel * SPACING( MAX(ABS(a),ABS(b)) ) )
END FUNCTION real_eq

SUBROUTINE check(string,error,total_error)
  CHARACTER(LEN=*) :: string
  INTEGER :: error, total_error
  IF (error .LT. 0) THEN
     total_error=total_error+1
     WRITE(*,*) string, " FAILED"
  ENDIF
  RETURN
END SUBROUTINE check
