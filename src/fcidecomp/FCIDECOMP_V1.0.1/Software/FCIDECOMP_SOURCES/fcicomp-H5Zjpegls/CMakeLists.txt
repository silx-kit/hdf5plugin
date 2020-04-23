# $Id: CMakeLists.txt 778 2016-03-09 07:56:29Z delaunay $
# =============================================================
# 
# PROJECT  : FCI_COMPRESSION 
# 
# AUTHOR   : THALES Services
# 
# Copyright 2015 EUMETSAT
# 
# =============================================================
# HISTORY :
# 
# VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
#
# END-HISTORY
# =============================================================
 
# Build the HDF5 JPEG-LS compression-decompression filter.
#
# To provide the module with a hint about where to find your FCICOMP_JPEGLS
# installation, you can set the environment variable FCICOMP_JPEGLS_ROOT. The
# Find module will then look in this path when searching for CharLS
# paths, and libraries.
#
# The module is build with compiler settings in
# ${FCICOMP_RESOURCES_DIR}/compilerFlags.cmake.
# 
# Unit test settings are in
# ${FCICOMP_RESOURCES_DIR}/unitTestSettings.cmake.
# 

cmake_minimum_required (VERSION 2.8.12)
PROJECT (H5ZJPEGLS C)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/Version.cmake)

MESSAGE("")
MESSAGE("**************************")
MESSAGE("   ${PROJECT_NAME}")
MESSAGE("**************************")
MESSAGE("")

# FCICOMP_ROOT should be set
IF (NOT FCICOMP_ROOT)
  # Otherwise try to set the FCICOMP_ROOT to a default value
  SET (FCICOMP_ROOT ${H5ZJPEGLS_SOURCE_DIR}/..)
ENDIF (NOT FCICOMP_ROOT)

# Define where to find the FCICOMP_COMMON source code
SET (FCICOMP_COMMON_SOURCE_DIR ${FCICOMP_ROOT}/fcicomp-common)

#---------------------------------------------------------------------------
# Optional Settings
#---------------------------------------------------------------------------

# Force building with shared libraries instead of static
SET(BUILD_SHARED_LIBS ON)
# Build with logging enable?
OPTION(LOGGING "Enable logging" ON)
# Build the unit tests?
OPTION(BUILD_TESTING "Build unit tests" ON)
OPTION(MEMORY_CHECK "Enable memory check" OFF)
OPTION(COVERAGE_TESTING "Enable coverage testing" OFF)

#-----------------------------------------------------------------------------
# Define some CMake variables for use later in the project
#-----------------------------------------------------------------------------
SET (FCICOMP_CMAKE_MODULE_PATH ${FCICOMP_ROOT}/cmake/modules)
SET (FCICOMP_RESOURCES_DIR     ${FCICOMP_ROOT}/cmake/config)

#-----------------------------------------------------------------------------
# Set the path for external cmake modules
#-----------------------------------------------------------------------------
SET (CMAKE_MODULE_PATH ${FCICOMP_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

#-----------------------------------------------------------------------------
# FCICOMP-JPEGLS Library Settings
# Users can set the FCICOMP_JPEGLS_ROOT variable which is copied to ENV{FCICOMP_JPEGLS_ROOT}
#-----------------------------------------------------------------------------

# Copy the FCICOMP_JPEGLS_ROOT variable retrieved from the cmake command line option 
#  -DFCICOMP_JPEGLS_ROOT into the ENV{FCICOMP_JPEGLS_ROOT} variable used by the FIND_PACKAGE(FCICOMP_JPEGLS_ROOT) command.
SET (ENV{FCICOMP_JPEGLS_ROOT} ${FCICOMP_JPEGLS_ROOT})

# Find FCICOMP_JPEGLS includes and library
FIND_PACKAGE (FCICOMP_JPEGLS REQUIRED)
INCLUDE_DIRECTORIES (${FCICOMP_JPEGLS_INCLUDE_DIR})

#-----------------------------------------------------------------------------
# HDF5 Library Settings
# Users can set the HDF5_ROOT variable which is copied to ENV{HDF5_ROOT}
# ENV{HDF5_ROOT} is used in by FindHDF5.cmake at the call of FIND_PACKAGE(HDF5_ROOT)
#-----------------------------------------------------------------------------

# Copy the HDF5_ROOT variable retrieved from the cmake command line option 
#  -DHDF5_ROOT into the ENV{HDF5_ROOT} variable used by the FIND_PACKAGE(HDF5) command.
IF (HDF5_ROOT)
  SET (ENV{HDF5_ROOT} ${HDF5_ROOT})
ENDIF (HDF5_ROOT)

# Find HDF5 includes and library
FIND_PACKAGE (HDF5 REQUIRED)

# Set the HDF5 include directories if they have been found
IF (HDF5_INCLUDE_DIRS)
  INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIRS})
ENDIF (HDF5_INCLUDE_DIRS)

#-----------------------------------------------------------------------------
# Set the compiler specific flags 
#-----------------------------------------------------------------------------
INCLUDE (${FCICOMP_RESOURCES_DIR}/compilerFlags.cmake)

#-----------------------------------------------------------------------------
# Set the unit test settings 
#-----------------------------------------------------------------------------
INCLUDE (${FCICOMP_RESOURCES_DIR}/unitTestSettings.cmake)

#-----------------------------------------------------------------------------
# System inspection
#-----------------------------------------------------------------------------

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES("stdlib.h"       HAVE_STDLIB_H)
CHECK_INCLUDE_FILES("stdio.h"        HAVE_STDIO_H)
CHECK_INCLUDE_FILES("stdarg.h"       HAVE_STDARG_H)
CHECK_INCLUDE_FILES("strings.h"      HAVE_STRINGS_H)

#-----------------------------------------------------------------------------
# Define the rpath settings for the H5Zjpegls library
#-----------------------------------------------------------------------------

# =========== NOTE ON THE HDF5 PLUGIN INSTALLATION DIRECTORY =================
# 
# The default directory for an HDF5 filter plugin library is defined on
# UNIX-like systems as:
#
# /usr/local/hdf5/lib/plugin
#
# It is recommended for users to set the CMAKE_INSTALL_PREFIX variable
# point to the /usr/local directory.
# 
# The default path can be overwritten by a user with the
# HDF5_PLUGIN_PATH environment variable.
#
# ============================================================================

# Set the default installation directory
IF (NOT CMAKE_INSTALL_PREFIX)
  SET (CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF (NOT CMAKE_INSTALL_PREFIX)

# Add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# The RPATH to be used when installing, but only if it's not a system directory
LIST (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/hdf5/lib/plugin" isSystemDir)
IF ("${isSystemDir}" STREQUAL "-1")
   SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/hdf5/lib/plugin")
ENDIF ("${isSystemDir}" STREQUAL "-1")

#-----------------------------------------------------------------------------
# Define H5Zjpegls library
#-----------------------------------------------------------------------------

# Define the name of the library
SET (TARGET_LIBRARY_NAME H5Zjpegls)

# Create the list of source code files
SET (H5ZJPEGLS_SRCS 
    ${H5ZJPEGLS_SOURCE_DIR}/src/H5Zjpegls.c
    ${FCICOMP_COMMON_SOURCE_DIR}/src/fcicomp_log.c
)

# Include the "include" directory
INCLUDE_DIRECTORIES (${H5ZJPEGLS_SOURCE_DIR}/include
    ${FCICOMP_COMMON_SOURCE_DIR}/include)

# Create the H5Zjpegls library
ADD_LIBRARY (${TARGET_LIBRARY_NAME} SHARED ${H5ZJPEGLS_SRCS})
TARGET_LINK_LIBRARIES (${TARGET_LIBRARY_NAME} PRIVATE ${FCICOMP_JPEGLS_LIBRARIES})

# Set the version and soversion of the library
# this creates the dynamic links
SET_TARGET_PROPERTIES (${TARGET_LIBRARY_NAME} PROPERTIES 
    VERSION ${H5ZJPEGLS_VERSION}
    SOVERSION ${H5ZJPEGLS_SOVERSION})

    
#-----------------------------------------------------------------------------
# Logging settings
#-----------------------------------------------------------------------------

# Define the default logging level
SET (DEFAULT_LOGGING_LEVEL "ERROR_SEVERITY")

# Define the default logging level depending on the debug mode
IF("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    SET (DEFAULT_LOGGING_LEVEL "DEBUG_SEVERITY")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")


# Enable logging 
IF (LOGGING OR LOGGING_LEVEL)
  SET (EXTRA_COMPILER_FLAGS "-DLOGGING ${EXTRA_COMPILER_FLAGS}")
  
  # Set the logging level
  IF (NOT LOGGING_LEVEL)
    # Set the default logging level
    SET (LOGGING_LEVEL ${DEFAULT_LOGGING_LEVEL})
  ENDIF (NOT LOGGING_LEVEL)

  # Set the logging level
  SET (EXTRA_COMPILER_FLAGS "-DLOGGING_LEVEL=${DEFAULT_LOGGING_LEVEL} ${EXTRA_COMPILER_FLAGS}")

ENDIF (LOGGING OR LOGGING_LEVEL)

#-----------------------------------------------------------------------------
# Set extra compiler flags
#-----------------------------------------------------------------------------

# Test Coverage Settings
IF (COVERAGE_TESTING)
  SET (EXTRA_COMPILER_FLAGS "-fprofile-arcs -ftest-coverage ${EXTRA_COMPILER_FLAGS}")
  SET (EXTRA_LINK_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage ${EXTRA_LINK_FLAGS_DEBUG} ")
ENDIF (COVERAGE_TESTING)

# Add extra compiler flags if any
IF (EXTRA_COMPILER_FLAGS)
  SET_TARGET_PROPERTIES(${TARGET_LIBRARY_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_COMPILER_FLAGS})
ENDIF (EXTRA_COMPILER_FLAGS)

# Add extra link flags if any
IF (EXTRA_LINK_FLAGS_DEBUG)
  SET_TARGET_PROPERTIES(${TARGET_LIBRARY_NAME} PROPERTIES LINK_FLAGS_DEBUG ${EXTRA_LINK_FLAGS_DEBUG})
ENDIF (EXTRA_LINK_FLAGS_DEBUG)

#-----------------------------------------------------------------------------
# Define the installed components for the H5Zjpegls library
#-----------------------------------------------------------------------------

# Define the PROJECT_NAME_LOWER variable
STRING (TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Define the include and lib install dir
SET (INCLUDE_INSTALL_DIR include/)
SET (LIBRARY_INSTALL_DIR hdf5/lib/plugin/)

# Install the library
INSTALL (TARGETS ${TARGET_LIBRARY_NAME}
  EXPORT "${PROJECT_NAME_LOWER}-targets"
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
)

#  Install include headers
INSTALL (FILES ${H5ZJPEGLS_SOURCE_DIR}/include/H5Zjpegls.h 
  DESTINATION ${INCLUDE_INSTALL_DIR})

#-----------------------------------------------------------------------------
# Create the package
#-----------------------------------------------------------------------------

SET (PROJECT_VERSION ${H5ZJPEGLS_VERSION})
INCLUDE (${FCICOMP_RESOURCES_DIR}/packageConfig.cmake)

#-----------------------------------------------------------------------------
# Unit tests
#-----------------------------------------------------------------------------
IF (BUILD_TESTING)

    # Define the unit tests directory
    SET (TEST_DIR ${H5ZJPEGLS_SOURCE_DIR}/test)
    # Define where the source codes of the unit test are located
    SET (TEST_SOURCE_DIR ${TEST_DIR}/src)
    # Define the directory where are located the test data
    SET (TEST_DATA_DIR ${TEST_DIR}/data)

    #-------------------------------------------------------------------------
    # Define the executables test programs
    #-------------------------------------------------------------------------
    
    # H5Zjpegls_test
    ADD_EXECUTABLE (H5Zjpegls_test
        ${TEST_SOURCE_DIR}/H5Zjpegls_test.c
        ${TEST_SOURCE_DIR}/H5Zjpegls_encode_nominal.c
        ${TEST_SOURCE_DIR}/H5Zjpegls_encode_rgba_nominal.c
        ${TEST_SOURCE_DIR}/H5Zjpegls_encode_cannot_apply.c
        ${TEST_SOURCE_DIR}/H5Zjpegls_decode_nominal.c
	)
    TARGET_LINK_LIBRARIES (H5Zjpegls_test ${HDF5_LIBRARIES})

    #-------------------------------------------------------------------------
    # Define the test image
    #-------------------------------------------------------------------------

    # sample
    # ------

    # Define the test image name, size and bpp
    SET (TEST_SAMPLE_IMAGE_NAME "sample")
    SET (TEST_SAMPLE_IMAGE_SIZE "200;100")
    SET (TEST_SAMPLE_IMAGE_BPP "16")
    # Define the reference uncompressed image file
    SET (REF_SAMPLE_RAW_FILE "${TEST_DATA_DIR}/${TEST_SAMPLE_IMAGE_NAME}_ref.raw")
    # Define the reference compressed image file
    SET (REF_SAMPLE_HDF5_FILE "${TEST_DATA_DIR}/${TEST_SAMPLE_IMAGE_NAME}_ref.h5")

    # RGBA_sample
    # -----------

    # Define the test RGBA image name, size and bpp
    SET (TEST_RGBA_SAMPLE_IMAGE_NAME "RGBA_sample")
    SET (TEST_RGBA_SAMPLE_IMAGE_SIZE "60;40")
    SET (TEST_RGBA_SAMPLE_IMAGE_BPP "8")
    SET (TEST_RGBA_SAMPLE_IMAGE_COMPONENTS "4")
    # Define the reference uncompressed image file
    SET (REF_RGBA_SAMPLE_RAW_FILE "${TEST_DATA_DIR}/${TEST_RGBA_SAMPLE_IMAGE_NAME}_ref.raw")
    # Define the reference compressed image file
    SET (REF_RGBA_SAMPLE_HDF5_FILE "${TEST_DATA_DIR}/${TEST_RGBA_SAMPLE_IMAGE_NAME}_ref.h5")

    # noise_sample
    # -----------

    # Define the test RGBA image name, size and bpp
    SET (TEST_NOISE_SAMPLE_IMAGE_NAME "noise_sample")
    SET (TEST_NOISE_SAMPLE_IMAGE_SIZE "32;32")
    SET (TEST_NOISE_SAMPLE_IMAGE_BPP "16")
    # Define the reference uncompressed image file
    SET (REF_NOISE_SAMPLE_RAW_FILE "${TEST_DATA_DIR}/${TEST_NOISE_SAMPLE_IMAGE_NAME}_ref.raw")
    # Define the reference compressed image file
    SET (REF_NOISE_SAMPLE_HDF5_FILE "${TEST_DATA_DIR}/${TEST_NOISE_SAMPLE_IMAGE_NAME}_ref.h5")

    #-------------------------------------------------------------------------
    # Define the unit test macro for this module
    #-------------------------------------------------------------------------

    # -------------------------------------------
    # Define the unit test macro for this module
    MACRO (ADD_H5ZJPEGLS_UNIT_TEST testname input_file output_file reference_file)
      IF (MEMORY_CHECK)
	# If memory check is enable, do not run the tests using cmake
	# Run the test directly
	ADD_TEST (
          NAME ${testname}
          COMMAND H5Zjpegls_test ${testname} ${input_file} ${output_file} ${ARGN}
	  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	  )
      ELSE (MEMORY_CHECK)

	# If LOGGING is not enable, do not compare the stderr
	IF (LOGGING)
	  SET(TEST_REF_ERR ${TEST_DATA_DIR}/${testname}_ref.err)
	ENDIF(LOGGING)

	# If memory check is disable, also run file comparisons
	ADD_TEST (
	  NAME ${testname}
	  COMMAND "${CMAKE_COMMAND}"
	  -D "TEST_NAME=${testname}"
	  -D "TEST_PROGRAM=$<TARGET_FILE:H5Zjpegls_test>"
	  -D "TEST_ARGS:STRING=${testname};${input_file};${output_file};${ARGN}"
	  -D "TEST_FOLDER=${PROJECT_BINARY_DIR}"
	  -D "TEST_EXPECT=0"
	  -D "TEST_REF_ERR=${TEST_REF_ERR}"
          -D "TEST_OUTPUT_FILE=${output_file}"
          -D "TEST_REF_OUTPUT_FILE=${reference_file}"
	  -P "${FCICOMP_RESOURCES_DIR}/runTestCompare.cmake"
	)
      ENDIF (MEMORY_CHECK)
      # Add the building directory to the HDF5_PLUGIN_PATH environment variable
      SET_PROPERTY(TEST ${testname} PROPERTY ENVIRONMENT "HDF5_PLUGIN_PATH=${H5ZJPEGLS_BINARY_DIR}:$ENV{HDF5_PLUGIN_PATH}" )
    ENDMACRO (ADD_H5ZJPEGLS_UNIT_TEST)
    

    # # ---------------------------------------------
    # # Define the h5ls test macro for this module
    # # This macro compare the output of h5ls to reference results
    # # The second argument is the dependancy on a previous test (which generates the h5 file)
    
    # # Try to find h5ls
    # FIND_PROGRAM (H5LS h5ls
    #   HINTS ENV HDF5_ROOT
    #   PATH_SUFFIXES bin Bin)

    # MACRO (ADD_H5LS_CMP_UNIT_TEST h5filename testdep)
    #   # If memory check is enable, do not run the tests
    #   IF (NOT MEMORY_CHECK)
    # 	# Test with h5ls -v -r
    # 	IF (H5LS AND HDF5_FOUND)
    # 	  # Get the HDF5 libray path: the first element in the list HDF5_LIBRARIES
    # 	  LIST (GET HDF5_LIBRARIES 0 HDF5_LIBRARY)
    # 	  # Get the directory of the HDF5 library
    # 	  GET_FILENAME_COMPONENT (HDF5_LIBRARY_DIR ${HDF5_LIBRARY} DIRECTORY)
    # 	  # Add a test with h5ls
    # 	  ADD_TEST (
    # 	    NAME h5ls:${h5filename}
    # 	    COMMAND "${CMAKE_COMMAND}"
    # 	    -D "TEST_PROGRAM=${H5LS}"
    # 	    -D "TEST_ARGS:STRING=-v;-r;${h5filename}"
    # 	    -D "TEST_FOLDER=${PROJECT_BINARY_DIR}"
    # 	    -D "TEST_OUTPUT=${h5filename}.h5ls"
    # 	    -D "TEST_EXPECT=0"
    # 	    -D "TEST_REFERENCE=${TEST_DATA_DIR}/${h5filename}.h5ls"
    # 	    -P "${FCICOMP_RESOURCES_DIR}/runTest.cmake"
    # 	    )
    # 	  # Set the LD_LIBRARY_PATH for H5LS to find the hdf5 library 
    # 	  SET_PROPERTY (TEST h5ls:${h5filename} PROPERTY ENVIRONMENT
    # 	    "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HDF5_LIBRARY_DIR}")
    # 	ENDIF (H5LS AND HDF5_FOUND)
    # 	SET_TESTS_PROPERTIES (h5ls:${h5filename} PROPERTIES DEPENDS testdep)
    # 	# Compare the h5 file with the reference
    # 	ADD_TEST (
    # 	  NAME cmp:${h5filename}
    # 	  COMMAND ${CMAKE_COMMAND} 
    # 	  -E compare_files 
    # 	  ${h5filename}
    # 	  ${TEST_DATA_DIR}/${h5filename}
    # 	  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    # 	  )
    # 	SET_TESTS_PROPERTIES (cmp:${h5filename} PROPERTIES DEPENDS testdep)
    #   ENDIF (NOT MEMORY_CHECK)
    # ENDMACRO (ADD_H5LS_CMP_UNIT_TEST)
    
    #-------------------------------------------------------------------------
    # Define the unit tests
    #-------------------------------------------------------------------------
    
    # ---------------------------
    # Test H5ZjpeglsEncodeNominal
        
    # Define the name of the compressed file at the output of the
    # nominal compression test
    SET (TEST_OUTPUT_FILE "${TEST_SAMPLE_IMAGE_NAME}.h5")
    # Add the unit test: run H5Zjpegls_test with the H5ZjpeglsEncodeNominal
    # test and the raw image at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsEncodeNominal
      ${REF_SAMPLE_RAW_FILE}
      ${TEST_OUTPUT_FILE}
      ${REF_SAMPLE_HDF5_FILE}
      ${TEST_SAMPLE_IMAGE_SIZE} ${TEST_SAMPLE_IMAGE_BPP})
    # Add the h5ls unit test: compare the h5ls of the h5 file to the reference 
    # ADD_H5LS_CMP_UNIT_TEST (${TEST_OUTPUT_FILE} H5ZjpeglsEncodeNominal)

    # -------------------------------
    # Test H5ZjpeglsEncodeRGBANominal
        
    # Define the name of the compressed file at the output of the
    # nominal compression test
    SET (TEST_OUTPUT_FILE "${TEST_RGBA_SAMPLE_IMAGE_NAME}.h5")
    # Add the unit test: run H5Zjpegls_test with the H5ZjpeglsEncodeRGBANominal
    # test and the raw image at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsEncodeRGBANominal
      ${REF_RGBA_SAMPLE_RAW_FILE}
      ${TEST_OUTPUT_FILE}
      ${REF_RGBA_SAMPLE_HDF5_FILE}
      ${TEST_RGBA_SAMPLE_IMAGE_SIZE} ${TEST_RGBA_SAMPLE_IMAGE_COMPONENTS} ${TEST_RGBA_SAMPLE_IMAGE_BPP})
    # Add the h5ls unit test: compare the h5ls of the h5 file to the reference 
    # ADD_H5LS_CMP_UNIT_TEST (${TEST_OUTPUT_FILE} H5ZjpeglsEncodeRGBANominal)

    # -------------------------------
    # Test H5ZjpeglsEncodeIneffective

    # Define the name of the file at the output of the test
    SET (TEST_OUTPUT_FILE "${TEST_NOISE_SAMPLE_IMAGE_NAME}.h5")
    # Add the unit test: run H5Zjpegls_test with the H5ZjpeglsEncodeNominal
    # test and the raw image at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsEncodeIneffective
      ${REF_NOISE_SAMPLE_RAW_FILE}
      ${TEST_OUTPUT_FILE}
      ${REF_NOISE_SAMPLE_HDF5_FILE}
      ${TEST_NOISE_SAMPLE_IMAGE_SIZE} ${TEST_NOISE_SAMPLE_IMAGE_BPP})
    # Add the h5ls unit test: compare the h5ls of the h5 file to the reference 
    # ADD_H5LS_CMP_UNIT_TEST (${TEST_OUTPUT_FILE} H5ZjpeglsEncodeIneffective)

    # -------------------------------
    # Test H5ZjpeglsEncodeCannotApply

    # Define the name of the file at the output of the test
    SET (TEST_OUTPUT_FILE "testH5ZjpeglsEncodeCannotApply.h5")
    # Add the unit test: run H5Zjpegls_test with the H5ZjpeglsEncodeCannotApply
    # test and the raw image at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsEncodeCannotApply 
      ${REF_SAMPLE_RAW_FILE} 
      ${TEST_OUTPUT_FILE}
      ${TEST_DATA_DIR}/testH5ZjpeglsEncodeCannotApply_ref.h5
      ${TEST_SAMPLE_IMAGE_SIZE} ${TEST_SAMPLE_IMAGE_BPP})
    # Add the h5ls unit test: compare the h5ls of the h5 file to the reference 
    # ADD_H5LS_CMP_UNIT_TEST (${TEST_OUTPUT_FILE} H5ZjpeglsEncodeCannotApply)
        
    # ---------------------------
    # Test H5ZjpeglsDecodeNominal
        
    # Define the name of the decompressed file at the output of the
    # nominal decompression test
    SET (TEST_OUTPUT_FILE "${TEST_SAMPLE_IMAGE_NAME}.raw")
    # Add the unit test: run H5Zjpegls_decode_test with the
    # H5ZjpeglsDecodeNominal test and the hdf5 file at the input, and
    # compare the decompressed output file with the reference raw file
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsDecodeNominal
      ${REF_SAMPLE_HDF5_FILE}
      ${TEST_OUTPUT_FILE}
      ${REF_SAMPLE_RAW_FILE})

    # -------------------------------
    # Test H5ZjpeglsDecodeRGBANominal
        
    # Define the name of the decompressed file at the output of the
    # nominal decompression test
    SET (TEST_RGBA_SAMPLE_OUTPUT_FILE "${TEST_RGBA_SAMPLE_IMAGE_NAME}.raw")
    # Add the unit test: run H5Zjpegls_decode_test with the H5ZjpeglsDecodeNominal
    # test and the  hdf5 file at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsDecodeRGBANominal
      ${REF_RGBA_SAMPLE_HDF5_FILE}
      ${TEST_RGBA_SAMPLE_OUTPUT_FILE}
      ${REF_RGBA_SAMPLE_RAW_FILE})

    # -------------------------------
    # Test H5ZjpeglsDecodeIneffective

    # Define the name of the compressed file at the output of the
    # nominal compression test
    SET (TEST_OUTPUT_FILE "${TEST_NOISE_SAMPLE_IMAGE_NAME}.h5")
    # Add the unit test: run H5Zjpegls_decode_test with the H5ZjpeglsDecodeNominal
    # test and the hdf5 file at the input
    ADD_H5ZJPEGLS_UNIT_TEST (H5ZjpeglsDecodeIneffective
      ${REF_NOISE_SAMPLE_HDF5_FILE}
      ${TEST_OUTPUT_FILE}
      ${REF_NOISE_SAMPLE_RAW_FILE})


ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Print the configuration summary
#-----------------------------------------------------------------------------

MESSAGE("")
MESSAGE("-----------------------------------------------------------------")
MESSAGE("Configuration Summary of ${PROJECT_NAME}:")
MESSAGE("")
MESSAGE(STATUS "Building Shared Libraries:  ${BUILD_SHARED_LIBS}")
IF(CMAKE_PREFIX_PATH)
  MESSAGE(STATUS "CMake Prefix Path:          ${CMAKE_PREFIX_PATH}")
ENDIF()
IF(CMAKE_INSTALL_PREFIX)
  MESSAGE(STATUS "CMake Install Prefix:       ${CMAKE_INSTALL_PREFIX}")
ENDIF()

MESSAGE("")
MESSAGE("Options:")
MESSAGE(STATUS "Logging:                    ${LOGGING}")
IF(LOGGING)
  MESSAGE(STATUS "Logging level:              ${LOGGING_LEVEL}")
ENDIF()

  
MESSAGE("")
MESSAGE("Compiler:")
MESSAGE(STATUS "Build Type:                 ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_C_COMPILER:           ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "CMAKE_C_FLAGS:              ${CMAKE_C_FLAGS}")
IF("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
  MESSAGE(STATUS "CMAKE_C_FLAGS_DEBUG:        ${CMAKE_C_FLAGS_DEBUG}")
ENDIF()
IF("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
  MESSAGE(STATUS "CMAKE_C_FLAGS_RELEASE:      ${CMAKE_C_FLAGS_RELEASE}")
ENDIF()
IF(EXTRA_COMPILER_FLAGS)
  MESSAGE(STATUS "EXTRA_COMPILER_FLAGS:       ${EXTRA_COMPILER_FLAGS}")
ENDIF()
MESSAGE(STATUS "Linking against:            ${FCICOMP_JPEGLS_LIBRARIES}")

MESSAGE("")
MESSAGE("Tests:")
MESSAGE(STATUS "Tests Enabled:              ${BUILD_TESTING}")
MESSAGE(STATUS "Coverage testing:           ${COVERAGE_TESTING}")
MESSAGE(STATUS "Memory check:               ${MEMORY_CHECK}")

MESSAGE("-----------------------------------------------------------------")
MESSAGE("")
