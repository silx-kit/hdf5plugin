# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2019 European Synchrotron Radiation Facility
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# ###########################################################################*/
"""This module provides compiled shared libraries for their use as HDF5 filters
under windows, MacOS and linux."""

__authors__ = ["V.A. Sole", "H. Payno", "T. Vincent"]
__license__ = "MIT"
__date__ = "30/09/2019"

import ctypes as _ctypes
from glob import glob as _glob
import logging as _logging
import os as _os
import sys as _sys

import h5py as _h5py


_logger = _logging.getLogger(__name__)


# Check _version module to avoid importing from source
try:
    from ._version import __date__ as date  # noqa
    from ._version import version, version_info, hexversion, strictversion  # noqa
except ImportError:
    raise RuntimeError(
        "Do NOT use %s from its sources: build it and use the built version" %
        _os.path.basename(_os.path.dirname(_os.path.abspath(__file__))))


PLUGINS_PATH = _os.path.abspath(
        _os.path.join(_os.path.dirname(__file__), 'plugins'))
"""Path where HDF5 filter plugins are stored"""


# IDs of provided filters
BLOSC = 32001
"""Blosc filter ID"""

BSHUF = 32008
"""Bitshuffle filter ID"""

LZ4 = 32004
"""LZ4 filter ID"""

FILTERS = {'blosc': BLOSC, 'bshuf': BSHUF, 'lz4': LZ4}
"""Mapping of filter name to HDF5 filter ID for available filters"""

# compression_opts

_blosc_shuffle = {
    None: 0,
    'none': 0,
    'byte': 1,
    'bit': 2,
    }

_blosc_compression = {
    'blosclz': 0,
    'lz4': 1,
    'lz4hc': 2,
    # Not built 'snappy': 3,
    'zlib': 4,
    'zstd': 5,
    }


def blosc_options(level=9, shuffle='byte', compression='blosclz'):
    """Prepare h5py.Group.create_dataset's compression and compression_opts arguments for using blosc filter.

    :param int level:
        Compression level from 0 no compression to 9 maximum compression.
        Default: 9.
    :param str shuffle:
        - `none` or None: no shuffle
        - `byte`: byte-wise shuffle
        - `bit`: bit-wise shuffle.
    :param str compression:
        `blosclz` (default), `lz4`, `lz4hc`, `zlib`, `zstd`
    :returns: compression and compression_opts arguments for h5py.Group.create_dataset
    :rtype: dict
    """
    level = int(level)
    assert 0 <= level <= 9
    shuffle = _blosc_shuffle[shuffle]
    compression = _blosc_compression[compression]
    return {'compression_opts': (0, 0, 0, 0, level, shuffle, compression),
            'compression': BLOSC}


def bshuf_options(nelems=0, lz4=True):
    """Prepare h5py.Group.create_dataset's compression and compression_opts arguments for using bitshuffle filter.

    :param int nelems:
        The number of elements per block.
        Default: 0 (for about 8kB per block).
    :param bool lz4:
        Whether to use LZ4 compression or not as part of the filter.
        Default: True
    :returns: compression and compression_opts arguments for h5py.Group.create_dataset
    :rtype: dict
    """
    nelems = int(nelems)
    assert nelems % 8 == 0

    lz4_enabled = 2 if lz4 else 0

    return {'compression_opts': (nelems, lz4_enabled),
            'compression': BSHUF}

def lz4_options(nbytes=0):
    """Prepare h5py.Group.create_dataset's compression and compression_opts arguments for using lz4 filter.

    :param int nelems:
        The number of bytes per block.
        Default: 0 (for 1GB per block).
    :returns: compression and compression_opts arguments for h5py.Group.create_dataset
    :rtype: dict
    """
    nbytes = int(nbytes)
    assert 0 <= nbytes <= 0x7E000000
    return {'compression_opts': (nbytes,),
            'compression': LZ4}


def _init_filters():
    """Initialise and register HDF5 filters with h5py

    Generator of tuples: (filename, library handle)
    """
    hdf5_version = _h5py.h5.get_libversion()

    for name, filter_id in FILTERS.items():
        # Check if filter is already loaded (not on buggy HDF5 versions)
        if (1, 8, 20) <= hdf5_version < (1, 10) or hdf5_version >= (1, 10, 2):
            if _h5py.h5z.filter_avail(filter_id):
                _logger.warning("%s filter already loaded, skip it.", name)
                continue

        # Load DLL
        filename = _glob(_os.path.join(PLUGINS_PATH, 'libh5' + name + '*'))[0]
        lib = _ctypes.CDLL(filename)

        if _sys.platform.startswith('win'):
            # Use register_filter function to register filter
            lib.register_filter.restype = _ctypes.c_int
            retval = lib.register_filter()
        else:
            # Use init_filter function to initialize DLL and register filter
            lib.init_filter.argtypes = [_ctypes.c_char_p]
            lib.init_filter.restype = _ctypes.c_int
            if _sys.version_info[0] >= 3:
                libname = bytes(_h5py.h5z.__file__, encoding='utf-8')
            else:
                libname = _h5py.h5z.__file__
            retval = lib.init_filter(libname)

        if retval < 0:
            _logger.error("Cannot initialize filter %s: %d", name, retval)
            continue

        yield filename, lib


_filters = dict(_init_filters())  # Store loaded filters
