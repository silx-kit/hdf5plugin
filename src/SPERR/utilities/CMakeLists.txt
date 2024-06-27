add_executable( show_version show_version.cpp )
target_link_libraries( show_version )

add_executable( sperr3d sperr3d.cpp )
target_link_libraries( sperr3d PUBLIC SPERR PUBLIC CLI11::CLI11 )

add_executable( sperr3d_trunc sperr3d_trunc.cpp )
target_link_libraries( sperr3d_trunc PUBLIC SPERR PUBLIC CLI11::CLI11 )

add_executable( sperr2d sperr2d.cpp )
target_link_libraries( sperr2d PUBLIC SPERR PUBLIC CLI11::CLI11 )

