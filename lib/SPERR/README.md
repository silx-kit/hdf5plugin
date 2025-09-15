[![clang-format](https://github.com/NCAR/SPERR/actions/workflows/clang-format.yml/badge.svg)](https://github.com/NCAR/SPERR/actions/workflows/clang-format.yml)
[![unit-test](https://github.com/NCAR/SPERR/actions/workflows/unit-test.yml/badge.svg)](https://github.com/NCAR/SPERR/actions/workflows/unit-test.yml)
[![CodeQL](https://github.com/NCAR/SPERR/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/NCAR/SPERR/actions/workflows/codeql-analysis.yml)
[![DOI](https://zenodo.org/badge/225491235.svg)](https://zenodo.org/badge/latestdoi/225491235)


## Overview

SPERR (pronounced like *spur*) is a lossy compressor for scientific data (2D or 3D floating-point data, mostly produced by numerical simulations). 
SPERR has one of the highest coding efficiencies among popular lossy compressors, meaning that it usually uses the least amount of storage
to satisfy a prescribed error tolerance (e.g., a maximum point-wise error tolerance).


Under the hood, SPERR uses wavelet transforms, [SPECK](https://ieeexplore.ieee.org/document/1347192) coding, 
and a custom outlier coding algorithm in its compression pipeline. 
This combination gives SPERR flexibility to compress targetting different quality controls, namely 1) bit-per-pixel (BPP), 
2) peak signal-to-noise ratio (PSNR), and 3) point-wise error (PWE).
The name of SPERR stands for **SP**eck with **ERR**or bounding.

## Quick Build
SPERR requires 1) a working C++ compiler and 2) CMake tools to build. On a Unix-like system,
the build steps are the following:

```bash
git clone https://github.com/NCAR/SPERR.git     # clone the repo
mkdir SPERR/build                               # create the build directory
cd SPERR/build                                  # enter the build directory
cmake ..                                        # use cmake to configure the project
cmake -DUSE_OMP=ON ..                           # Optional: enable OpenMP on 3D volumes.
cmake -DCMAKE_INSTALL_PREFIX=/my/install/dir .. # Optional: specify a directory to install SPERR. The default is /usr/local .
cmake -DCMAKE_CXX_STANDARD=17 ..                # Optional: use C++17 rather than C++20. The code is slightly faster with C++20.
make -j 8                                       # build the project
ctest .                                         # run unit tests, which should have 100% tests passed
make install                                    # install the library and CLI tools to a specified directory.
```

## Plugin for HDF5
SPERR is available as a *dynamically loaded plugin* for HDF5 with a registered ID of `32028`.
This plugin is available at this [repo](https://github.com/NCAR/H5Z-SPERR).

## Wrapper for Fortran
A Fortran wrapper for SPERR has also been created by [ofmla](https://github.com/ofmla) 
at this [repo](https://github.com/ofmla/fortran-sperr).

## Documentation

SPERR documentation is hosted on Github [Wiki](https://github.com/NCAR/SPERR/wiki) pages. To get started, one might want to
[build SPERR from source](https://github.com/NCAR/SPERR/wiki/Build-SPERR-From-Source) and explore compression and decompression
utilities for [3D and 2D](https://github.com/NCAR/SPERR/wiki/CLI%3A-3D-and-2D-Compression-and-Decompression-Utilities) inputs.
One can also use [spack](https://spack.io/) to install SPERR by a single command `spack install sperr`.
Finally, a collection of canonical scientific data sets is available at [SDRBench](https://sdrbench.github.io/) for testing and evaluation purposes.

SPERR also provides programming [API in C++ and C](https://github.com/NCAR/SPERR/wiki#sperr-c-api).

## Publication

If SPERR benefits your work, please kindly cite [this publication](https://ieeexplore.ieee.org/document/10177487):
```Tex
@INPROCEEDINGS{10177487,
  author={Li, Shaomeng and Lindstrom, Peter and Clyne, John},
  booktitle={2023 IEEE International Parallel and Distributed Processing Symposium (IPDPS)}, 
  title={Lossy Scientific Data Compression With SPERR}, 
  year={2023},
  pages={1007-1017},
  doi={10.1109/IPDPS54959.2023.00104}}
```
(Author's copy is available [here](https://vast.ucar.edu/pdfs/SPERR_IPDPS.pdf).)

## Presentations
- FZ Workshop Hands-on: Feb 15 2024, Sarasota, FL. ([handout and examples](https://vast.ucar.edu/pdfs/Li_FZ2024.pdf))
- SC'23 Tutorial on lossy scientific data compression: Nov 13 2023, Denver CO. ([slides](https://vast.ucar.edu/pdfs/Li_SC23_Slides.pdf))
- IPDPS'23 Lossy Scientific Data Compression With SPERR: May 18 2023, St. Petersburg, FL. ([slides](https://vast.ucar.edu/pdfs/Li_IPDPS23_Slides.pdf))
