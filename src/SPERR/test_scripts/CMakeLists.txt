add_executable(        sperr_helper sperr_helper_unit_test.cpp )
target_link_libraries( sperr_helper PUBLIC SPERR gtest_main )

add_executable(        bitstream bitstream_unit_test.cpp )
target_link_libraries( bitstream PUBLIC SPERR gtest_main )

add_executable(        dwt dwt_unit_test.cpp )
target_link_libraries( dwt PUBLIC SPERR gtest_main )

add_executable(        speck_int speck_int_unit_test.cpp )
target_link_libraries( speck_int PUBLIC SPERR gtest_main )

add_executable(        outlier_coder outlier_coder_unit_test.cpp )
target_link_libraries( outlier_coder PUBLIC SPERR gtest_main )

add_executable(        speck2d_flt speck2d_flt_unit_test.cpp )
target_link_libraries( speck2d_flt PUBLIC SPERR gtest_main )

add_executable(        speck3d_flt speck3d_flt_unit_test.cpp )
target_link_libraries( speck3d_flt PUBLIC SPERR gtest_main )

add_executable(        sperr3d_omp sperr3d_omp_unit_test.cpp )
target_link_libraries( sperr3d_omp PUBLIC SPERR gtest_main )

add_executable(        stream_tools stream_tools_unit_test.cpp )
target_link_libraries( stream_tools PUBLIC SPERR gtest_main )

include(GoogleTest)
gtest_discover_tests( sperr_helper )
gtest_discover_tests( bitstream )
gtest_discover_tests( dwt     )
gtest_discover_tests( speck_int )
gtest_discover_tests( outlier_coder )
gtest_discover_tests( speck2d_flt )
gtest_discover_tests( speck3d_flt )
gtest_discover_tests( sperr3d_omp )
gtest_discover_tests( stream_tools )
