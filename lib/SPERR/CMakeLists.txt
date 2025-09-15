# Tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/index.html

cmake_minimum_required(VERSION 3.14)

project(SPERR VERSION 0.8.2 LANGUAGES CXX DESCRIPTION "Lossy Scientific Compression with SPERR")

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD "20" CACHE STRING "Choose the C++ Standard to use." FORCE)
    set_property(CACHE CMAKE_CXX_STANDARD PROPERTY STRINGS "20" "17")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

include(CMakeDependentOption)
#
# Test system endian. SPERR only works on little endian machines,
# which are pretty much all machines on the market in 2020.
#
include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
 message(FATAL_ERROR "CMake detects that the target machine is BIG ENDIAN,\
                      which SPERR cannot work on!")
else()
 message(STATUS "Little endian machine, good!")
endif()

option( BUILD_SHARED_LIBS "Build shared SPERR library" ON )
option( BUILD_UNIT_TESTS "Build unit tests using GoogleTest" ON )
option( BUILD_CLI_UTILITIES "Build a set of command line utilities" ON )
option( USE_OMP "Use OpenMP parallelization on 3D volumes" OFF )
option( SPERR_PREFER_RPATH "Set RPATH; this can fight with package managers so turn off when building for them" ON )
mark_as_advanced(FORCE SPERR_PREFER_RPATH)

if(USE_OMP)
  find_package(OpenMP REQUIRED)
  if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found! (${OpenMP_CXX_LIB_NAMES}-${OpenMP_CXX_VERSION}: ${OpenMP_CXX_FLAGS})")
  else()
    message(FATAL_ERROR "OpenMP NOT found! Try turning it off to continue configuring SPERR!")
  endif()
endif()

#
# Gather git commit SHA1
#
find_package(Git)
execute_process(COMMAND
  ${GIT_EXECUTABLE} describe --always --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND
  ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
  # Note: when git version 2.22 is more common, then we can use the following command
  #       to retrieve the current branch name.
  # ${GIT_EXECUTABLE} branch --show-current
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Repository Git Branch: ${GIT_BRANCH}, SHA1: ${GIT_SHA1}")

#
# configure a header file to pass some of the CMake settings to the source code
#
configure_file("${CMAKE_SOURCE_DIR}/SperrConfig.h.in" SperrConfig.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


if(SPERR_PREFER_RPATH)
#
# Basically always use full rpath when installing.
# These specifications need to be placed before defining any targets.
#
# Kitware documentation on CMake's handle or rpath:
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
# and a good discussion on rpath topics:
# https://stackoverflow.com/questions/32469953/why-is-cmake-designed-so-that-it-removes-runtime-path-when-installing
#
set( CMAKE_SKIP_BUILD_RPATH             FALSE )
set( CMAKE_BUILD_WITH_INSTALL_RPATH     FALSE )
set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE  )
endif()


#
# Compile the SPERR library
#
add_subdirectory( src )

#
# Build command line utilities
#
if( BUILD_CLI_UTILITIES )
  # Download and build CLI11
  #
  include(FetchContent)

  # Control internal options of CLI11
  #
  set( CLI11_PRECOMPILED ON CACHE INTERNAL "Save precompiled CLI11")
  set( CLI11_SINGLE_FILE OFF CACHE INTERNAL "Don't use single file CLI11")
  FetchContent_Declare( cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG        6c7b07a878ad834957b98d0f9ce1dbe0cb204fc9 # v2.4.2
  )
  FetchContent_MakeAvailable(cli11)

  add_subdirectory( utilities ${CMAKE_BINARY_DIR}/bin )
endif()


#
# Fetch Google Test: https://google.github.io/googletest/quickstart-cmake.html
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
    message (STATUS "Fetching the latest GoogleTest Framework, CMake version >= 3.24")
    FetchContent_Declare( googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip
      DOWNLOAD_EXTRACT_TIMESTAMP NEW )
  else()
    message (STATUS "Fetching the latest GoogleTest Framework, CMake version < 3.24")
    FetchContent_Declare( googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip )
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows
  #
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  # Copy test data sets to the build directory
  #
  file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

  enable_testing() # calling this function before adding subdirectory to enable 
                   # invoking ctest from the top-level build directory.
  add_subdirectory( test_scripts )
endif()


#
# Start installation using GNU installation rules
#
include( GNUInstallDirs )

# Install the SPERR library
#
if( BUILD_SHARED_LIBS )
  install( TARGETS SPERR LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
else()
  install( TARGETS SPERR ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
endif()

# Install utilities
#
if( BUILD_CLI_UTILITIES )
  install( TARGETS show_version sperr3d sperr2d sperr3d_trunc
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
endif()

# Add a pkg-config file, also copy over SperrConfig.h
#
configure_file("${CMAKE_SOURCE_DIR}/SPERR.pc.in" SPERR.pc @ONLY)
install( FILES "${CMAKE_BINARY_DIR}/SPERR.pc" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
install( FILES "${CMAKE_BINARY_DIR}/SperrConfig.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )

