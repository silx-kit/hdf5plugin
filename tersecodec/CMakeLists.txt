cmake_minimum_required(VERSION 3.12)
project(tersecodec)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 ")


cmake_policy(SET CMP0074 NEW)

set(HDF5_ROOT "$ENV{HDF5_ROOT}")
if(NOT HDF5_ROOT)
    set(HDF5_ROOT "$ENV{CONDA_PREFIX}")
endif()

set(TERSE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
include_directories(${TERSE_INCLUDE_DIR})


find_package(HDF5 REQUIRED COMPONENTS C CXX HL)
if(HDF5_FOUND)
    message(STATUS "HDF5 include dirs: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "HDF5 library dirs: ${HDF5_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "HDF5 not found in ${HDF5_ROOT}")
endif()

include_directories(${HDF5_INCLUDE_DIRS})


set(PLUGIN_NAME "h5terse")
add_library(${PLUGIN_NAME} SHARED src/filter.cpp)


target_include_directories(${PLUGIN_NAME} PUBLIC ${TERSE_INCLUDE_DIR})
target_compile_options(${PLUGIN_NAME} PRIVATE -fPIC)
target_link_libraries(${PLUGIN_NAME} ${HDF5_LIBRARIES})


set_target_properties(${PLUGIN_NAME} PROPERTIES
    OUTPUT_NAME "${PLUGIN_NAME}"
    PREFIX "lib"
    SUFFIX ".so"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${HDF5_LIBRARY_DIRS}"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Install to standard HDF5 plugin location
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${HDF5_ROOT}" CACHE PATH "Default install path" FORCE)
endif()

install(TARGETS ${PLUGIN_NAME}
    LIBRARY DESTINATION lib/hdf5/plugins
)