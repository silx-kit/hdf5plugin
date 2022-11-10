cmake_minimum_required(VERSION 3.9)

# Fail immediately if not using an out-of-source build
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif ()

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
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_CMAKEDIR lib/cmake/h5z_zfp)

#------------------------------------------------------------------------------#
# Required packages
#------------------------------------------------------------------------------#
# Find HDF5, relies on HDF5_DIR or HDF5_ROOT being set in environment.
set(COMPONENTS C)
if (FORTRAN_INTERFACE)
  list(APPEND COMPONENTS Fortran)
endif ()
find_package(HDF5 REQUIRED COMPONENTS ${COMPONENTS})

# Find ZFP, relies on ZFP_DIR or ZFP_ROOT being set in environment.
find_package(ZFP REQUIRED CONFIG)

#------------------------------------------------------------------------------#
# Add source
#------------------------------------------------------------------------------#
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

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
