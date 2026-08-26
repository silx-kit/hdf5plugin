cmake_minimum_required(VERSION 3.28)

project(H5Z_htj2k VERSION 1.0.0 LANGUAGES C
  DESCRIPTION "HDF5 plugin for High-throughput JPEG2000 compression")

# ── C standard ────────────────────────────────────────────────────────────────
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Build type (default Release) ──────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# ── Decode-only mode (disables encoder) ──────────────────────────────────────
option(DECODE_ONLY "Build the htj2k filter in decode-only mode (encoder disabled)" OFF)

# ── Optional explicit backend library location ────────────────────────────────
# Point these at a directory containing the backend's headers/shared
# libraries when they aren't found via CMake config, pkg-config, or standard
# system paths.
set(BACKEND_INCLUDE_DIR "" CACHE PATH
  "Directory containing the selected backend's headers")
set(BACKEND_LIB_DIR "" CACHE PATH
  "Directory containing the selected backend's shared libraries")

if((BACKEND_INCLUDE_DIR AND NOT BACKEND_LIB_DIR) OR (BACKEND_LIB_DIR AND NOT BACKEND_INCLUDE_DIR))
  message(FATAL_ERROR
    "BACKEND_INCLUDE_DIR and BACKEND_LIB_DIR must be set together.\n"
    "  BACKEND_INCLUDE_DIR = '${BACKEND_INCLUDE_DIR}'\n"
    "  BACKEND_LIB_DIR     = '${BACKEND_LIB_DIR}'")
endif()

# ── GNU install directories (lib, include, bin) ──────────────────────────────
include(GNUInstallDirs)

# ── Dependencies ──────────────────────────────────────────────────────────────

# HDF5
find_package(HDF5 REQUIRED COMPONENTS C)
message(STATUS "Found HDF5 version ${HDF5_VERSION}: ${HDF5_C_LIBRARIES}")

# ── Hidden default visibility (Linux/macOS) ──────────────────────────────────
# Only H5PLget_plugin_type/H5PLget_plugin_info stay exported -- H5PLextern.h
# marks them default-visibility -- everything else, including the statically
# linked OpenJPH symbols, is hidden. Set before building OpenJPH (below) so
# its object files are compiled hidden too.
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ── OpenJPH backend ──────────────────────────────────────────────────────────
if(BACKEND_INCLUDE_DIR AND BACKEND_LIB_DIR)
  # Use a pre-built OpenJPH instead of the vendored source
  find_library(_openjph_lib
    NAMES openjph libopenjph
    PATHS ${BACKEND_LIB_DIR}
    NO_DEFAULT_PATH
  )
  if(NOT _openjph_lib)
    message(FATAL_ERROR
      "Could not find the OpenJPH library in BACKEND_LIB_DIR='${BACKEND_LIB_DIR}'")
  endif()

  message(STATUS "Using OpenJPH backend (prebuilt: ${_openjph_lib})")

  set(_backend_include_dirs ${BACKEND_INCLUDE_DIR})
  set(_backend_libs ${_openjph_lib})
else()
  # Build the vendored openjph library only as a static lib, and skip its
  # own apps/tests
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(OJPH_ENABLE_TIFF_SUPPORT OFF CACHE BOOL "" FORCE)
  set(OJPH_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(OJPH_BUILD_EXECUTABLES OFF CACHE BOOL "" FORCE)
  set(OJPH_BUILD_STREAM_EXPAND OFF CACHE BOOL "" FORCE)
  set(OJPH_BUILD_FUZZER OFF CACHE BOOL "" FORCE)

  add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/vendored/OpenJPH
    ${CMAKE_CURRENT_BINARY_DIR}/openjph
    EXCLUDE_FROM_ALL
  )

  message(STATUS "Using OpenJPH backend (vendored: ${CMAKE_CURRENT_SOURCE_DIR}/vendored/OpenJPH)")

  set(_backend_include_dirs)
  set(_backend_libs openjph)
endif()

# ── Shared library (HDF5 plugin) ──────────────────────────────────────────────
add_library(H5Z_htj2k SHARED
  src/H5Zhtj2k.c
  src/backend_openjph.cpp
)

target_include_directories(H5Z_htj2k
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${HDF5_INCLUDE_DIRS}
    ${_backend_include_dirs}
)

target_link_libraries(H5Z_htj2k
  PRIVATE
    ${HDF5_C_LIBRARIES}
    ${_backend_libs}
    ${CMAKE_DL_LIBS}
)

if(DECODE_ONLY)
  target_compile_definitions(H5Z_htj2k PRIVATE H5Z_HTJ2K_FILTER_DECODE_ONLY)
  message(STATUS "HTJ2K filter built in decode-only mode")
endif()

set_target_properties(H5Z_htj2k PROPERTIES
  OUTPUT_NAME h5zhtj2k
  POSITION_INDEPENDENT_CODE ON
)

# ── Windows: export only the HDF5 plugin entry points ────────────────────────
if(WIN32)
  target_sources(H5Z_htj2k PRIVATE src/H5Zhtj2k.def)
endif()

# ── Install ───────────────────────────────────────────────────────────────────

# HDF5 plugin directory (HDF5 looks for plugins under <libdir>/hdf5/plugin)
set(HDF5_PLUGIN_INSTALL_DIR
  "${CMAKE_INSTALL_LIBDIR}/hdf5/plugin"
  CACHE PATH "Install directory for the HDF5 plugin")

install(TARGETS H5Z_htj2k
  LIBRARY DESTINATION ${HDF5_PLUGIN_INSTALL_DIR}
  RUNTIME DESTINATION ${HDF5_PLUGIN_INSTALL_DIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
