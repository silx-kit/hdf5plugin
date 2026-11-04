cmake_minimum_required(VERSION 3.15)
project(pyterse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(pybind11 CONFIG REQUIRED)

message(STATUS "pybind11_INCLUDE_DIRS: ${pybind11_INCLUDE_DIRS}")
message(STATUS "pybind11_VERSION: ${pybind11_VERSION}")

pybind11_add_module(pyterse src/pyterse.cpp)

target_include_directories(pyterse PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(pyterse PROPERTIES OUTPUT_NAME "pyterse")
set_target_properties(pyterse PROPERTIES PREFIX "")


if(MSVC)
    target_compile_options(pyterse PRIVATE /O2)
else()
    target_compile_options(pyterse PRIVATE -O3 -fpermissive)
endif()


if(APPLE)
    set_target_properties(pyterse PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()

target_link_libraries(pyterse PRIVATE pybind11::pybind11)

# Install the module
install(TARGETS pyterse
        LIBRARY DESTINATION .
        ARCHIVE DESTINATION .
        RUNTIME DESTINATION .
)
