cmake_minimum_required(VERSION 3.9)

# Fail immediately if not using an out-of-source build
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif ()

if (POLICY CMP0074)
  # find_package() uses <PackageName>_ROOT variables.
  cmake_policy (SET CMP0074 NEW)
endif ()

if (POLICY CMP0083)
  # To control generation of Position Independent Executable (PIE) or not,
  # some flags are required at link time.
  cmake_policy (SET CMP0083 NEW)
endif ()

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

#------------------------------------------------------------------------------#
# Parse version number from H5Zzfp_version.h
#------------------------------------------------------------------------------#
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_MAJOR")
string(REGEX REPLACE "^.*MAJOR " "" H5Z_ZFP_VERSION_MAJOR "${H5Z_ZFP_H}")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_MINOR")
string(REGEX REPLACE "^.*MINOR " "" H5Z_ZFP_VERSION_MINOR "${H5Z_ZFP_H}")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_PATCH")
string(REGEX REPLACE "^.*PATCH " "" H5Z_ZFP_VERSION_PATCH "${H5Z_ZFP_H}")
set(H5Z_ZFP_VERSION "${H5Z_ZFP_VERSION_MAJOR}.${H5Z_ZFP_VERSION_MINOR}.${H5Z_ZFP_VERSION_PATCH}")

project(H5Z_ZFP VERSION ${H5Z_ZFP_VERSION} LANGUAGES C)

# setup common output folders
include (cmake/HDFMacros.cmake)
SET_HDF_BUILD_TYPE()
DEFAULT_FOLDERS()

#------------------------------------------------------------------------------#
# Compile options.
#------------------------------------------------------------------------------#
option(FORTRAN_INTERFACE "Enable the Fortran interface" ON)
if (FORTRAN_INTERFACE)
  enable_language(Fortran)
endif ()
#------------------------------------------------------------------------------#
# Some boilerplate to setup nice output directories
#------------------------------------------------------------------------------#
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_CMAKEDIR lib/cmake/h5z_zfp)

#------------------------------------------------------------------------------#
# Required packages
#------------------------------------------------------------------------------#
# Find HDF5, relies on HDF5_DIR or HDF5_ROOT being set in environment.
HDF5_SUPPORT ()
if (NOT HDF5_FOUND)
  message (FATAL_ERROR "HDF5 is Required for h5z_zfp")
endif ()

# Find MPI depending on if HDF5 needs MPI.
if ((HDF5_IS_PARALLEL OR HDF5_ENABLE_PARALLEL) AND ("${HDF5_VERSION}" VERSION_EQUAL "1.14.0"))
  find_package(MPI REQUIRED COMPONENTS C)
endif()

# Find ZFP, relies on ZFP_DIR or ZFP_ROOT being set in environment.
find_package(ZFP REQUIRED CONFIG)
if (NOT ZFP_FOUND)
  message (FATAL_ERROR "ZFP is Required for h5z_zfp")
endif ()

#------------------------------------------------------------------------------#
# Add source
#------------------------------------------------------------------------------#
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

#-----------------------------------------------------------------------------
# Dashboard and Testing Settings
#-----------------------------------------------------------------------------
option (BUILD_TESTING "Build h5z-zfp Unit Testing" OFF)
if (BUILD_TESTING)
  set (DART_TESTING_TIMEOUT 1200 CACHE STRING
       "Timeout in seconds for each test (default 1200=20minutes)")
  enable_testing ()
  include (CTest)

  add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif ()

#------------------------------------------------------------------------------#
# Packaging
#------------------------------------------------------------------------------#
# Install h5z_zfp-config.cmake and h5z_zfp-config-version.cmake
set(H5Z_ZFP_CONFIG_IN  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/h5z_zfp-config.cmake.in)
set(H5Z_ZFP_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/h5z_zfp-config.cmake)
configure_file(${H5Z_ZFP_CONFIG_IN} ${H5Z_ZFP_CONFIG_OUT} @ONLY)
set(H5Z_ZFP_CONFIG_VERSION_IN  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/h5z_zfp-config-version.cmake.in)
set(H5Z_ZFP_CONFIG_VERSION_OUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/h5z_zfp-config-version.cmake)
configure_file(${H5Z_ZFP_CONFIG_VERSION_IN} ${H5Z_ZFP_CONFIG_VERSION_OUT} @ONLY)
install(FILES ${H5Z_ZFP_CONFIG_OUT}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(FILES ${H5Z_ZFP_CONFIG_VERSION_OUT}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
