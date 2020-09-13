cmake_minimum_required(VERSION 2.8.10)
project(jpeg_hdf5)
include(ExternalProject)

# Want to see more output from make
set(CMAKE_VERBOSE_MAKEFILE ON)

# options
option(BUILD_TESTS
    "Build test programs form the blosc filter" ON)

option(BUILD_PLUGIN
    "Build dynamically loadable plugin for HDF5 version > 1.8.11" ON)
if(BUILD_PLUGIN)
    set(PLUGIN_INSTALL_PATH "/usr/local/hdf5/lib/plugin" CACHE PATH 
      "Where to install the dynamic HDF5-plugin")
endif(BUILD_PLUGIN)

# sources
set(SOURCES src/jpeg_h5filter.c)
set(PLUGIN_SOURCES src/jpeg_h5filter.c src/jpeg_h5plugin.c )

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
    find_package(JPEG REQUIRED)
endif(MSVC)
include_directories(${HDF5_INCLUDE_DIRS})
include_directories(${JPEG_INCLUDE_DIRS})


add_library(jpeg_filter_shared SHARED ${SOURCES})
set_target_properties(
  jpeg_filter_shared PROPERTIES OUTPUT_NAME jpeg_h5filter)
target_link_libraries(jpeg_filter_shared ${JPEG_LIBRARIES} ${HDF5_LIBRARIES})

if(BUILD_PLUGIN)
    add_library(jpeg_plugin_shared SHARED ${PLUGIN_SOURCES})
    set_target_properties(
      jpeg_plugin_shared PROPERTIES OUTPUT_NAME jpeg_h5plugin)
    target_link_libraries(jpeg_plugin_shared jpeg_filter_shared ${JPEG_LIBRARIES} ${HDF5_LIBRARIES})

    install(TARGETS jpeg_plugin_shared DESTINATION ${PLUGIN_INSTALL_PATH} COMPONENT HDF5_FILTER_DEV)
endif(BUILD_PLUGIN)

# install
install(FILES src/jpeg_h5filter.h DESTINATION include COMPONENT HDF5_FILTER_DEV)
install(TARGETS jpeg_filter_shared DESTINATION lib COMPONENT HDF5_FILTER_DEV)


# test
message("LINK LIBRARIES='jpeg_filter_shared ${HDF5_LIBRARIES}'")
if(BUILD_TESTS)
    enable_testing()
    add_executable(example src/example.c)
    target_link_libraries(example jpeg_filter_shared ${JPEG_LIBRARIES} ${HDF5_LIBRARIES})
    add_test(test_hdf5_filter example)
endif(BUILD_TESTS)
