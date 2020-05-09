# H5Z-ZFP

A highly flexible floating point and integer
compression plugin for the HDF5 library using ZFP compression.

[![Build Status](https://travis-ci.org/LLNL/H5Z-ZFP.svg?branch=master)](https://travis-ci.org/LLNL/H5Z-ZFP)
[![Documentation Status](https://readthedocs.org/projects/h5z-zfp/badge/?version=latest)](http://h5z-zfp.readthedocs.io)
[![codecov](https://codecov.io/gh/LLNL/H5Z-ZFP/branch/master/graph/badge.svg)](https://codecov.io/gh/LLNL/H5Z-ZFP)

For information about ZFP compression and the BSD-Licensed ZFP
library, see...

- https://computation.llnl.gov/projects/floating-point-compression
- https://github.com/LLNL/zfp

For information about HDF5 filter plugins, see...

- https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters

This H5Z-ZFP plugin supports ZFP versions 0.5.0 through 0.5.5.

This plugin uses the [*registered*](https://support.hdfgroup.org/services/filters.html#zfp)
HDF5 plugin filter id 32013

The  HDF5  filter  plugin  code here is also part of the Silo library.
However, we have made an  effort to also support  it as a  stand-alone
package  due  to  the  likely  broad  appeal  and  utility  of the ZFP
compression library.

This plugin supports all modes of the ZFP compression library, *rate*,
*accuracy*, *precision* and *expert* and *lossless*. It supports 1, 2 and
3 dimensional datasets of single and double precision integer and floating
point data. It can be applied to HDF5 datasets of more than 3 dimensions
(or 4 dimensions for ZFP versions 0.5.5 and newer) as long as no more than 3
(or 4) dimensions of the HDF5 dataset chunking are of size greater than 1.

[**Full documentation**](http://h5z-zfp.readthedocs.io)
