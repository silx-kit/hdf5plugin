#
# The main H5Z-SPERR filter
#
add_library( h5z-sperr h5z-sperr.c
                       h5zsperr_helper.cpp
                       icecream.c
                       compactor.c)
target_include_directories( h5z-sperr PUBLIC ${HDF5_INCLUDE_DIR} 
                                      PUBLIC ${SPERR_INCLUDE_DIRS}
                                      PUBLIC ${CMAKE_SOURCE_DIR}/include )
target_link_libraries( h5z-sperr PUBLIC ${HDF5_LIBRARIES} PUBLIC PkgConfig::SPERR )

#
# Experimental clamping filter
#
add_library( h5z-clamp h5z-clamp.c )
target_include_directories( h5z-clamp PUBLIC ${HDF5_INCLUDE_DIR} )
target_link_libraries( h5z-clamp PUBLIC ${HDF5_LIBRARIES} )
