# $Id: CMakeLists.txt 823 2017-09-05 14:30:55Z delaunay $
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
# VERSION:1.0.2:NCR:FCICOMP-12:04/09/2017:Remove the signal handler
# VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
#
# END-HISTORY
# =============================================================
 
# Build the facade between CharLS software and the FCICOMP software.
#
# This CMake script makes use of the FindCHARLS.cmake script to find
# where CharLS library is installed on the system. This FindCHARLS.cmake
# script should be located at ${FCICOMP_ROOT}/cmake/modules. You
# can set the FCICOMP_ROOT environment variable. 
#
# 
# To provide the module with a hint about where to find your CharLS
# installation, you can set the environment variable CHARLS_ROOT. The
# Find module will then look in this path when searching for CharLS
# paths, and libraries.
#
# The facade is build with compiler settings in
# ${FCICOMP_RESOURCES_DIR}/compilerFlags.cmake.
# 
# Unit test settings are in
# ${FCICOMP_RESOURCES_DIR}/unitTestSettings.cmake.
# 

cmake_minimum_required (VERSION 2.8.12)

PROJECT (FCICOMP_JPEGLS C)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/Version.cmake)

MESSAGE ("")
MESSAGE("**************************")
MESSAGE ("   ${PROJECT_NAME}")
MESSAGE("**************************")
MESSAGE ("")

# FCICOMP_ROOT should be set
IF (NOT FCICOMP_ROOT)
  # Otherwise try to set the FCICOMP_ROOT to a default value
  SET (FCICOMP_ROOT ${FCICOMP_JPEGLS_SOURCE_DIR}/..)
ENDIF (NOT FCICOMP_ROOT)

#---------------------------------------------------------------------------
# Optional Settings
#---------------------------------------------------------------------------

# Build with shared libraries instead of static?
OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
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
# FindCHARLS.cmake is used to find CHARLS at the call of FIND_PACKAGE (CHARLS)
#-----------------------------------------------------------------------------
SET (CMAKE_MODULE_PATH ${FCICOMP_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

#-----------------------------------------------------------------------------
# CHARLS Library Settings
# Users can set the CHARLS_ROOT variable which is copied to ENV{CHARLS_ROOT}
# ENV{CHARLS_ROOT} is used in by FindCHARLS.cmake at the call of FIND_PACKAGE(CHARLS)
#-----------------------------------------------------------------------------

# Copy the CHARLS_ROOT variable retrieved from the cmake command line option 
#  -DCHARLS_ROOT into the ENV{CHARLS_ROOT} variable used by the FIND_PACKAGE(CHARLS_ROOT) command.
SET (ENV{CHARLS_ROOT} ${CHARLS_ROOT})

# Find CHARLS includes and library
FIND_PACKAGE (CHARLS REQUIRED)
INCLUDE_DIRECTORIES (${CHARLS_INCLUDE_DIR})

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
CHECK_INCLUDE_FILES("stdlib.h"    HAVE_STDLIB_H)
CHECK_INCLUDE_FILES("stdio.h"     HAVE_STDIO_H)
CHECK_INCLUDE_FILES("stdarg.h"    HAVE_STDARG_H)
CHECK_INCLUDE_FILES("strings.h"   HAVE_STRINGS_H)
CHECK_INCLUDE_FILES("signal.h"    HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES("setjmp.h"    HAVE_SETJMP_H)

#-----------------------------------------------------------------------------
# Define the rpath settings for the fcicomp_jpegls library
#-----------------------------------------------------------------------------

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

#-----------------------------------------------------------------------------
# Define fcicomp_jpegls library
#-----------------------------------------------------------------------------

# Define the name of the library
SET (TARGET_LIBRARY_NAME fcicomp_jpegls)

# Define where to find the FCICOMP_COMMON source code
SET (FCICOMP_COMMON_SOURCE_DIR ${FCICOMP_ROOT}/fcicomp-common)

# Create the list of source code files
SET (FCICOMP_JPEGLS_SRCS 
    ${FCICOMP_JPEGLS_SOURCE_DIR}/src/fcicomp_jpegls.c
    ${FCICOMP_COMMON_SOURCE_DIR}/src/fcicomp_log.c
)

# Include the directories
INCLUDE_DIRECTORIES (${FCICOMP_JPEGLS_SOURCE_DIR}/include 
    ${FCICOMP_COMMON_SOURCE_DIR}/include)

# Create the fcicomp_jpegls library
ADD_LIBRARY (${TARGET_LIBRARY_NAME} ${FCICOMP_JPEGLS_SRCS})
TARGET_LINK_LIBRARIES (${TARGET_LIBRARY_NAME} PRIVATE ${CHARLS_LIBRARIES})

# Set the version and soversion of the library
# this creates the dynamic links
SET_TARGET_PROPERTIES (${TARGET_LIBRARY_NAME} PROPERTIES 
    VERSION ${FCICOMP_JPEGLS_VERSION}
    SOVERSION ${FCICOMP_JPEGLS_SOVERSION})


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
# Define the installed components for the fcicomp_jpegls library
#-----------------------------------------------------------------------------

# Define the PROJECT_NAME_LOWER variable
STRING (TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Define the include and lib install dir
SET (INCLUDE_INSTALL_DIR include/)
SET (LIBRARY_INSTALL_DIR lib/)

# Install the library
INSTALL (TARGETS ${TARGET_LIBRARY_NAME}
  EXPORT "${PROJECT_NAME_LOWER}-targets"
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
)

# Install include headers
INSTALL (FILES ${FCICOMP_JPEGLS_SOURCE_DIR}/include/fcicomp_jpegls.h 
  DESTINATION ${INCLUDE_INSTALL_DIR})

#-----------------------------------------------------------------------------
# Create the package
#-----------------------------------------------------------------------------

SET (PROJECT_VERSION ${FCICOMP_JPEGLS_VERSION})
INCLUDE (${FCICOMP_RESOURCES_DIR}/packageConfig.cmake)

#-----------------------------------------------------------------------------
# Unit tests
#-----------------------------------------------------------------------------
IF (BUILD_TESTING)

    # Define the unit tests directory
    SET (TEST_DIR ${FCICOMP_JPEGLS_SOURCE_DIR}/test)
    # Define where the source codes of the unit test are located
    SET (TEST_SOURCE_DIR ${TEST_DIR}/src)
    # Define the directory where are located the test data
    SET (TEST_DATA_DIR ${TEST_DIR}/data)


    #-------------------------------------------------------------------------
    # Define the executables test programs
    #-------------------------------------------------------------------------
    
    # jpegls_test
    ADD_EXECUTABLE (jpegls_test
        ${TEST_SOURCE_DIR}/jpegls_test.c
        ${TEST_SOURCE_DIR}/jpegls_compress_nominal.c
        ${TEST_SOURCE_DIR}/jpegls_decompress_nominal.c
        ${TEST_SOURCE_DIR}/jpegls_compress_error_case.c
        ${TEST_SOURCE_DIR}/jpegls_read_header_error_case.c
        ${TEST_SOURCE_DIR}/jpegls_decompress_error_case.c
	)
    TARGET_LINK_LIBRARIES (jpegls_test ${TARGET_LIBRARY_NAME})
    SET_TARGET_PROPERTIES(jpegls_test PROPERTIES COMPILE_FLAGS "-DLOGGING")

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
    SET (REF_SAMPLE_JLS_FILE "${TEST_DATA_DIR}/${TEST_SAMPLE_IMAGE_NAME}_ref.jls")

    # noise_sample
    # -----------

    # Define the test RGBA image name, size and bpp
    SET (TEST_NOISE_SAMPLE_IMAGE_NAME "noise_sample")
    SET (TEST_NOISE_SAMPLE_IMAGE_SIZE "32;32")
    SET (TEST_NOISE_SAMPLE_IMAGE_BPP "16")
    # Define the reference uncompressed image file
    SET (REF_NOISE_SAMPLE_RAW_FILE "${TEST_DATA_DIR}/${TEST_NOISE_SAMPLE_IMAGE_NAME}_ref.raw")
    # Define the reference compressed image file
    SET (REF_NOISE_SAMPLE_JLS_FILE "${TEST_DATA_DIR}/${TEST_NOISE_SAMPLE_IMAGE_NAME}_ref.jls")

    #-------------------------------------------------------------------------
    # Define the unit test macros for this module
    #-------------------------------------------------------------------------

    # ------------------------------------------------------------------------
    # This macro run a unit test and perform a comparison of the stderr
    # and stdout to expected results
    MACRO (ADD_FCICOMP_JPEGLS_UNIT_TEST testname)
      IF (MEMORY_CHECK)
	# If memory check is enable, do not run the tests using cmake
	# Run the test directly
	ADD_TEST (
          NAME ${testname}
          COMMAND jpegls_test ${testname} ${ARGN}
	  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	  )
      ELSE (MEMORY_CHECK)
	# If memory check is disable, also run file comparisons
	
	# If LOGGING is not enable, do not compare the stderr
	IF (LOGGING)
	  SET(TEST_REF_ERR ${TEST_DATA_DIR}/${testname}_ref.err)
	ENDIF(LOGGING)

	ADD_TEST (
	  NAME ${testname}
	  COMMAND "${CMAKE_COMMAND}"
	  -D "TEST_NAME=${testname}"
	  -D "TEST_PROGRAM=$<TARGET_FILE:jpegls_test>"
	  -D "TEST_ARGS:STRING=${testname};${ARGN}"
	  -D "TEST_FOLDER=${PROJECT_BINARY_DIR}"
	  -D "TEST_EXPECT=0"
	  -D "TEST_REF_ERR=${TEST_REF_ERR}"
	  -P "${FCICOMP_RESOURCES_DIR}/runTestCompare.cmake"
	  )
      ENDIF (MEMORY_CHECK)
    ENDMACRO (ADD_FCICOMP_JPEGLS_UNIT_TEST)
    
    # ------------------------------------------------------------------------
    # This marco run a unit test and perform a comparison of the output
    # file with a reference file
    MACRO (ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST testname input_file output_file reference_file)
      
      IF (MEMORY_CHECK)
	# If memory check is enable, do not run the tests using cmake
	# Run the test directly
	ADD_TEST (
          NAME ${testname}
          COMMAND jpegls_test ${testname} ${input_file} ${output_file} ${ARGN}
	  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	  )

      ELSE (MEMORY_CHECK)
	# If memory check is disable, also run file comparisons
	ADD_TEST (
          NAME ${testname}
          COMMAND "${CMAKE_COMMAND}"
	  -D "TEST_NAME=${testname}"
          -D "TEST_PROGRAM=$<TARGET_FILE:jpegls_test>"
          -D "TEST_ARGS:STRING=${testname};${input_file};${output_file};${ARGN}"
          -D "TEST_FOLDER=${PROJECT_BINARY_DIR}"
          -D "TEST_EXPECT=0"
          -D "TEST_OUTPUT_FILE=${output_file}"
          -D "TEST_REF_OUTPUT_FILE=${reference_file}"
          -P "${FCICOMP_RESOURCES_DIR}/runTestCompare.cmake"
	  )
      ENDIF (MEMORY_CHECK)
    ENDMACRO (ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST)
    

    #-------------------------------------------------------------------------
    # Define the unit tests
    #-------------------------------------------------------------------------
    
    # --------------------------
    # Test jpeglsCompressNominal
        
    # Define the name of the compressed file at the output of the
    # nominal compression test
    SET (TEST_OUTPUT_FILE "${TEST_SAMPLE_IMAGE_NAME}.jls")
    # Add the unit test: run jpegls_test with the jpeglsCompressNominal
    # test and the raw image at the input, and compare the compressed
    # output file with the reference jls file
    ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST (jpeglsCompressNominal
      ${REF_SAMPLE_RAW_FILE} 
      ${TEST_OUTPUT_FILE}
      ${REF_SAMPLE_JLS_FILE} ${TEST_SAMPLE_IMAGE_SIZE} ${TEST_SAMPLE_IMAGE_BPP})

    # --------------------------
    # Test jpeglsDecompressNominal
        
    # Define the name of the decompressed file at the output of the
    # nominal decompression test
    SET (TEST_OUTPUT_FILE "${TEST_SAMPLE_IMAGE_NAME}.jls.raw")
    # Add the unit test: run jpegls_test with the
    # jpeglsDecompressNominal test and the jls image at the input, and
    # compare the decompressed output file with the reference raw file
    ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST (jpeglsDecompressNominal
      ${REF_SAMPLE_JLS_FILE} 
      ${TEST_OUTPUT_FILE}
      ${REF_SAMPLE_RAW_FILE})

    # --------------------------
    # Test jpeglsCompressNoise
        
    # Define the name of the compressed file at the output of the test
    SET (TEST_OUTPUT_FILE "${TEST_NOISE_SAMPLE_IMAGE_NAME}.jls")
    # Add the unit test: run jpegls_test with the jpeglsCompressNoise
    ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST (jpeglsCompressNoise
      ${REF_NOISE_SAMPLE_RAW_FILE}
      ${TEST_OUTPUT_FILE} 
      ${REF_NOISE_SAMPLE_JLS_FILE} ${TEST_NOISE_SAMPLE_IMAGE_SIZE} ${TEST_NOISE_SAMPLE_IMAGE_BPP})
      
    # --------------------------
    # Test jpeglsDecompressNoise
        
    # Define the name of the compressed file at the output of the test
    SET (TEST_OUTPUT_FILE "${TEST_NOISE_SAMPLE_IMAGE_NAME}.jls.raw")
    # Add the unit test: run jpegls_test with the jpeglsDecompressNoise
    ADD_FCICOMP_JPEGLS_CMP_UNIT_TEST (jpeglsDecompressNoise
      ${REF_NOISE_SAMPLE_JLS_FILE}
      ${TEST_OUTPUT_FILE} 
      ${REF_NOISE_SAMPLE_RAW_FILE})

    # -----------------------------
    # Test jpeglsCompressErrorCase
        
    # Define the name of the compressed file at the output of the test
    SET (TEST_OUTPUT_FILE "${TEST_SAMPLE_IMAGE_NAME}.jls")
    # Add the unit test: run jpegls_test with the jpeglsCompressErrorCase
    ADD_FCICOMP_JPEGLS_UNIT_TEST (jpeglsCompressErrorCase
      ${REF_SAMPLE_RAW_FILE} ${TEST_OUTPUT_FILE} ${TEST_SAMPLE_IMAGE_SIZE} ${TEST_SAMPLE_IMAGE_BPP})
    
    # --------------------------
    # Test jpeglsReadHeaderErrorCase
    # Add the unit test: run jpegls_test with the jpeglsReadHeaderErrorCase
    ADD_FCICOMP_JPEGLS_UNIT_TEST (jpeglsReadHeaderErrorCase)

    # --------------------------
    # Test jpeglsDecompressErrorCase
    # Add the unit test: run jpegls_test with the jpeglsDecompressErrorCase
    ADD_FCICOMP_JPEGLS_UNIT_TEST (jpeglsDecompressErrorCase ${REF_SAMPLE_JLS_FILE})

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
MESSAGE(STATUS "Linking against:            ${CHARLS_LIBRARIES}")

MESSAGE("")
MESSAGE("Tests:")
MESSAGE(STATUS "Tests Enabled:              ${BUILD_TESTING}")
MESSAGE(STATUS "Coverage testing:           ${COVERAGE_TESTING}")
MESSAGE(STATUS "Memory check:               ${MEMORY_CHECK}")

MESSAGE("-----------------------------------------------------------------")
MESSAGE("")
