cmake_minimum_required(VERSION 2.8.10)
cmake_policy(SET CMP0074 NEW)
project(blosc2_hdf5)
include(ExternalProject)

# options
option(BUILD_TESTS
    "Build test programs form the blosc2 filter" ON)

option(BUILD_PLUGIN
    "Build dynamically loadable plugin for HDF5 version > 1.8.11" ON)
if(BUILD_PLUGIN)
    set(PLUGIN_INSTALL_PATH "/usr/local/hdf5/lib/plugin" CACHE PATH
      "Where to install the dynamic HDF5-plugin")
endif(BUILD_PLUGIN)

set(BLOSC2_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/blosc2")
set(BLOSC2_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/blosc2")
set(BLOSC2_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${BLOSC2_INSTALL_DIR})

message("BLOSC2_PREFIX='${BLOSC2_PREFIX}'")
message("BLOSC2_INSTALL_DIR='${BLOSC2_INSTALL_DIR}'")
message("BLOSC2_CMAKE_ARGS='${BLOSC2_CMAKE_ARGS}'")
message("GIT_EXECUTABLE='${GIT_EXECUTABLE}'")

ExternalProject_Add(project_blosc2
  PREFIX ${BLOSC2_PREFIX}
  GIT_REPOSITORY https://github.com/Blosc/c-blosc2.git
  GIT_TAG main
  INSTALL_DIR ${BLOSC2_INSTALL_DIR}
  CMAKE_ARGS ${BLOSC2_CMAKE_ARGS}
)


# sources
set(SOURCES src/blosc2_filter.c)
set(PLUGIN_SOURCES src/blosc2_filter.c src/blosc2_plugin.c)

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
include_directories("/mnt/c/Users/sosca/CLionProjects/build/include")


# add blosc2 libraries
add_library(blosc2_shared SHARED IMPORTED)
set_property(TARGET blosc2_shared PROPERTY IMPORTED_LOCATION ${BLOSC2_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}blosc2${CMAKE_SHARED_LIBRARY_SUFFIX})
add_dependencies(blosc2_shared project_blosc2)
include_directories(${BLOSC2_INSTALL_DIR}/include)

add_library(blosc2_filter_shared SHARED ${SOURCES})
set_target_properties(
  blosc2_filter_shared PROPERTIES OUTPUT_NAME blosc2_filter)
target_link_libraries(blosc2_filter_shared blosc2_shared ${HDF5_LIBRARIES})

if(BUILD_PLUGIN)
    add_library(blosc2_plugin_shared SHARED ${PLUGIN_SOURCES})
    set_target_properties(
      blosc2_plugin_shared PROPERTIES OUTPUT_NAME H5Zblosc2)
    target_link_libraries(blosc2_plugin_shared blosc2_shared ${HDF5_LIBRARIES})

    install(TARGETS blosc2_plugin_shared DESTINATION ${PLUGIN_INSTALL_PATH} COMPONENT HDF5_FILTER_DEV)
endif(BUILD_PLUGIN)

# install
install(FILES src/blosc2_filter.h DESTINATION include COMPONENT HDF5_FILTER_DEV)
install(TARGETS blosc2_filter_shared DESTINATION lib COMPONENT HDF5_FILTER_DEV)


# add caterva library
add_library(CAT_LIBRARY SHARED IMPORTED)
set_target_properties(CAT_LIBRARY PROPERTIES IMPORTED_LOCATION "/mnt/c/Users/sosca/CLionProjects/build/libs/release/libcaterva.so")


# test
message("LINK LIBRARIES='blosc2_filter_shared ${HDF5_LIBRARIES}'")
if(BUILD_TESTS)
    enable_testing()
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
    add_executable(example executables/example.c)
    target_link_libraries(example blosc2_filter_shared ${HDF5_LIBRARIES} ${LIBS})
    add_test(test_hdf5_filter example)
    add_executable(test_blosc2_filter_read executables/test_blosc2_filter_read.c)
    target_link_libraries(test_blosc2_filter_read blosc2_filter_shared ${HDF5_LIBRARIES} ${LIBS} CAT_LIBRARY)
    add_test(test_hdf5_filter test_blosc2_filter_read)
endif(BUILD_TESTS)
