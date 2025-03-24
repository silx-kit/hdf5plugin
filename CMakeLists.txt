cmake_minimum_required(VERSION 3.14)

project(H5Z-SPERR VERSION 0.2.3 LANGUAGES C CXX DESCRIPTION "HDF5 plugin for SPERR compression")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

option( BUILD_SHARED_LIBS "Build shared libraries" ON )
option( BUILD_CLI_UTILITIES "Build a set of command line utilities" ON )
option( BUILD_UNIT_TESTS "Build unit tests using GoogleTest" OFF )
option( H5ZPLUGIN_PREFER_RPATH "Set RPATH; this can fight with package managers 
                                so turn off when building for them" ON )
mark_as_advanced(FORCE H5ZPLUGIN_PREFER_RPATH)

find_package(HDF5 REQUIRED COMPONENTS C)
message(STATUS "Found HDF5 Version: ${HDF5_VERSION}: ${HDF5_C_LIBRARIES}")

if(H5ZPLUGIN_PREFER_RPATH)
  set( CMAKE_SKIP_BUILD_RPATH             FALSE )
  set( CMAKE_BUILD_WITH_INSTALL_RPATH     FALSE )
  set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" )
  set( CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE  )
endif()

set (SPERR_INSTALL_DIR "SPERR INSTALL DIRECTORY" 
     CACHE STRING "where subdirectories such as lib and include are found")
list(APPEND CMAKE_PREFIX_PATH ${SPERR_INSTALL_DIR})
find_package(PkgConfig REQUIRED)
pkg_search_module(SPERR REQUIRED IMPORTED_TARGET GLOBAL SPERR)
if (SPERR_FOUND)
  message(STATUS "Found SPERR version ${SPERR_VERSION}: ${SPERR_LINK_LIBRARIES}")
endif()

add_subdirectory( src )

#
# Build command line utilities
#
if( BUILD_CLI_UTILITIES )
  add_subdirectory( utilities ${CMAKE_BINARY_DIR}/bin )
endif()

#
# Build unit tests
#
if( BUILD_UNIT_TESTS )
  # Control internal options of GoogleTest
  #
  set( INSTALL_GTEST OFF CACHE INTERNAL "Not install GoogleTest")
  set( BUILD_GMOCK ON CACHE INTERNAL "Build gmock")

  # Let's use the new mechanism to incorporate GoogleTest
  #
  include(FetchContent)
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    FetchContent_Declare( googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip
      DOWNLOAD_EXTRACT_TIMESTAMP NEW )
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows
  #
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing() # calling this function before adding subdirectory to enable 
                   # invoking ctest from the top-level build directory.
  add_subdirectory( test_scripts )
endif()

#
# Start installation using GNU installation rules
#
include( GNUInstallDirs )
install( TARGETS h5z-sperr h5z-clamp LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )

if( BUILD_CLI_UTILITIES )
  install( TARGETS generate_cd_values decode_cd_values
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
endif()
