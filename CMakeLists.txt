cmake_minimum_required(VERSION 3.15)
project(pyterse_and_tersecodec LANGUAGES CXX)


option(BUILD_PYTERSE "Build the pyterse Python extension" ON)
option(BUILD_TERSECODEC "Build the HDF5 filter library (tersecodec) plugin" ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


if(BUILD_PYTERSE)
  add_subdirectory(pyterse)
endif()

if(BUILD_TERSECODEC)
  add_subdirectory(tersecodec)
endif()
