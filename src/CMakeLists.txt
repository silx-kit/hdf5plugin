# Define headers.
set(h5z_zfp_headers
  H5Zzfp.h H5Zzfp_lib.h H5Zzfp_plugin.h H5Zzfp_props.h H5Zzfp_version.h)

#------------------------------------------------------------------------------#
# Static library
#------------------------------------------------------------------------------#
# Define source
set(h5z_zfp_lib_source
  ${h5z_zfp_headers}
  H5Zzfp_props_private.h
  H5Zzfp.c H5Zzfp_props.c)

if (FORTRAN_INTERFACE)
  list(APPEND h5z_zfp_lib_source H5Zzfp_props_f.F90)
endif ()

add_library(h5z_zfp_static STATIC ${h5z_zfp_lib_source})
target_compile_definitions(h5z_zfp_static PRIVATE H5Z_ZFP_AS_LIB)
target_include_directories(h5z_zfp_static
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(h5z_zfp_static PUBLIC zfp::zfp ${HDF5_LIBRARIES})
set_target_properties(h5z_zfp_static PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(h5z_zfp_static PROPERTIES OUTPUT_NAME h5zzfp)
install(TARGETS h5z_zfp_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

#------------------------------------------------------------------------------#
# Shared library
#------------------------------------------------------------------------------#
# Define source
set(h5z_zfp_plugin_source
  ${h5z_zfp_headers}
  H5Zzfp.c)

add_library(h5z_zfp_shared SHARED ${h5z_zfp_plugin_source})
target_include_directories(h5z_zfp_shared
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(h5z_zfp_shared PUBLIC zfp::zfp ${HDF5_LIBRARIES})
set_target_properties(h5z_zfp_shared PROPERTIES OUTPUT_NAME h5zzfp)
install(TARGETS h5z_zfp_shared
  ARCHIVE DESTINATION plugin
  LIBRARY DESTINATION plugin)

#------------------------------------------------------------------------------#
# Install header and module files.
#------------------------------------------------------------------------------#
install(FILES ${h5z_zfp_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (FORTRAN_INTERFACE)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/h5zzfp_props_f.mod
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif ()
