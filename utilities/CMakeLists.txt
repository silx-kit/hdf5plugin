add_executable( example-3d example-3d.c )
include_directories( example-3d ${HDF5_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include )
target_link_libraries( example-3d PUBLIC ${HDF5_LIBRARIES} PUBLIC "m" )

add_executable( example-2d example-2d.c )
include_directories( example-2d ${HDF5_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include )
target_link_libraries( example-2d PUBLIC ${HDF5_LIBRARIES} PUBLIC "m" )

add_executable( generate_cd_values generate_cd_values.c )
include_directories( generate_cd_values ${CMAKE_SOURCE_DIR}/include )
target_link_libraries( generate_cd_values PUBLIC "m" )

add_executable( decode_cd_values decode_cd_values.c )
include_directories( decode_cd_values ${CMAKE_SOURCE_DIR}/include )
target_link_libraries( decode_cd_values PUBLIC "m" )
