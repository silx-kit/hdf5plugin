Building this filter/example requires knowledge of the hdf5 and the zstd
installation.

For autotools configure, you must supply these using
    --with-hdf5 and --with-zstdlib.
Example (in the build folder):
    ../configure --with-hdf5=/temp/hdf5 --with-zstdlib=/temp/zstd

For CMake, you must supply the location of the cmake configuration files 
    in environment variables.
    In addition, CMake options "H5PL_BUILD_EXAMPLES" and "H5PL_BUILD_TESTING" must
    be set "ON" in order to build the example and run the tests.
Example:
    set(ENV{HDF5_ROOT} "/temp/hdf5/")
    set(ENV{ZSTD_ROOT} "/temp/zstd/")
    set(ENV{LD_LIBRARY_PATH} "/temp/zstd/lib:/temp/hdf5/lib")
    set(ADD_BUILD_OPTIONS "-DH5PL_BUILD_EXAMPLES:BOOL=ON -DH5PL_BUILD_TESTING:BOOL=ON")

    For non-cmake built hdf5 or zstd, use the location of the include/lib
    folders:
    set(ENV{HDF5_ROOT} "/temp/hdf5")
    set(ENV{ZSTD_ROOT} "/temp/zstd")
