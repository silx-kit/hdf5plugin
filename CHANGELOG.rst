2.3.1
-----

- Fixed support of wheel package version >= 0.35 (PR #82)
- Fixed typo in error log (PR #81)
- Continuous integration: Added check of package description (PR #80)
- Fixed handling of version info (PR #84)

2.3
---

- Added ZFP filter (PR #74, #77)
- Updated README (PR #76, #79)

2.2
---

- Added FCIDECOMP filter (PR #68, #71)

2.1.2
-----

- Fixed OpenMP compilation flag (PR #64)
- Fixed support of `wheel` package version >= 0.34 (PR #64)
- Continuous Integration: Run tests with python3 on macOS rather than python2. (PR #66)

2.1.1
-----

- Fixed `--native` build option on platform other than x86_64 (PR #62)
- Fixed build of the snappy C++11 library for blosc on macOS (PR #60)

2.1.0
-----

- Added `--openmp=[False|True]` build option to compile bitshuffle filter with OpenMP. (PR #51)
- Added `--sse2=[True|False]` build option to compile blosc and bitshuffle filters with SSE2 instructions if available. (PR #52)
- Added `--avx2=[True|False]` build option to compile blosc and bitshuffle filters with AVX2 instructions if available. (PR #52)
- Added `--native=[True|False]` build option to compile filters for native CPU architecture. This enables SSE2/AVX2 support for the bitshuffle filter if available. (PR #52)
- Added snappy compression to the blosc filter if C++11 is available (`--cpp11=[True|False]` build option). (PR #54)
- Improved wheel generation by using root_is_pure=True setting. (PR #49)

2.0.0
-----

- Added compression support for Linux and macOS
- Added blosc filter
- Added helper class (Blosc, Bitshuffle and LZ4) to ease providing compression arguments to h5py
- Added tests
- Updated documentation
- Building from source through setup.py
- No longer use the plugin mechanism via HDF5_PLUGIN_PATH environment variable

1.4.1
-----

- Support Python 3.7 under 64-bit windows

1.4.0
-----

- Manylinux support

1.3.1
-----

- Support Python 3.6 under 64-bit windows.

1.3.0
-----

- Add 64-bit manylinux version LZ4 filter plugin

- Add 64-bit manylinux version bitshuffle plugin

- Implement continuous imtegration testing


1.2.0
-----

- Add LZ4 filter plugin for MacOS

- Add bitshuffle plugin decompressor for MacOS

1.1.0
-----

- Add bitshuffle plugin.

- Document origin and license of the used sources.

1.0.1
-----

- Replace corrupted VS2015 64 bit dll.

1.0.0
-----

- Initial release with LZ4 filter plugin.
