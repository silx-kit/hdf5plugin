cmake_minimum_required(VERSION 2.8.10)
project(zstd_hdf5)

# options
set(PLUGIN_INSTALL_PATH "/usr/local/hdf5/lib/plugin" CACHE PATH
      "Where to install the dynamic HDF5-plugin")

# sources
set(SOURCES zstd_h5plugin.c)
set(PLUGIN_SOURCES zstd_h5plugin.c)

# dependencies
if(MSVC)
    # FindHDF5.cmake does not find Windows installations. Try to
    # use an environment variable instead until the official "find"
    # file can be updated for Windows.
    #
    # Note that you have to set this environment variable by hand.
    file(TO_CMAKE_PATH "$ENV{HDF5_DIR}" HDF5_HINT)
    set(HDF5_DIR ${HDF5_HINT} CACHE STRING "Path to HDF5 CMake config directory.")
    find_package(HDF5 REQUIRED HINTS ${HDF5_DIR})
else(MSVC)
    find_package(HDF5 REQUIRED)
endif(MSVC)
include_directories(${HDF5_INCLUDE_DIRS})

# HDF5 plugin as shared library
add_library(zstd_h5_plugin_shared SHARED ${PLUGIN_SOURCES})
set_target_properties(zstd_h5_plugin_shared PROPERTIES OUTPUT_NAME H5Zzstd)
target_link_libraries(zstd_h5_plugin_shared zstd ${HDF5_LIBRARIES})
install(TARGETS zstd_h5_plugin_shared DESTINATION ${PLUGIN_INSTALL_PATH} COMPONENT HDF5_FILTER_DEV)
