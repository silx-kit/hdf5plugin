add_executable(        compactor_test compactor_test.cpp )
target_link_libraries( compactor_test PUBLIC h5z-sperr GTest::gtest_main )

add_executable(        icecream_test icecream_test.cpp )
target_link_libraries( icecream_test PUBLIC h5z-sperr GTest::gtest_main )

add_executable(        helper_test h5zsperr_helper_test.cpp )
target_link_libraries( helper_test PUBLIC h5z-sperr GTest::gtest_main )

include(GoogleTest)
gtest_discover_tests( compactor_test )
gtest_discover_tests( icecream_test )
gtest_discover_tests( helper_test )
