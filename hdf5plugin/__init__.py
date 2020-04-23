# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2020 European Synchrotron Radiation Facility
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
__date__ = "24/04/2020"

import ctypes as _ctypes
from glob import glob as _glob
import logging as _logging
import os as _os
import sys as _sys
if _sys.version_info[0] >= 3:
    from collections.abc import Mapping as _Mapping
else:
    from collections import Mapping as _Mapping
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
BLOSC_ID = 32001
"""Blosc filter ID"""

BSHUF_ID = 32008
"""Bitshuffle filter ID"""

LZ4_ID = 32004
"""LZ4_ID filter ID"""

FCIDECOMP_ID = 32018
"""FCIDECOMP filter ID"""

FILTERS = {'blosc': BLOSC_ID,
           'bshuf': BSHUF_ID,
           'lz4': LZ4_ID,
           'fcidecomp': FCIDECOMP_ID,
           }
"""Mapping of filter name to HDF5 filter ID for available filters"""


try:
    _FilterRefClass = _h5py.filters.FilterRefBase
except AttributeError:
    class _FilterRefClass(_Mapping):
        """Base class for referring to an HDF5 and describing its options

        Your subclass must define filter_id, and may define a filter_options tuple.
        """
        filter_id = None
        filter_options = ()

        # Mapping interface supports using instances as **kwargs for compatibility
        # with older versions of h5py
        @property
        def _kwargs(self):
            return {
                'compression': self.filter_id,
                'compression_opts': self.filter_options
            }

        def __hash__(self):
            return hash((self.filter_id, self.filter_options))

        def __len__(self):
            return len(self._kwargs)

        def __iter__(self):
            return iter(self._kwargs)

        def __getitem__(self, item):
            return self._kwargs[item]


class Blosc(_FilterRefClass):
    """h5py.Group.create_dataset's compression and compression_opts arguments for using blosc filter.

    :param str cname:
        `blosclz`, `lz4` (default), `lz4hc`, `zlib`, `zstd`
        Optional: `snappy`, depending on compilation (requires C++11).
    :param int clevel:
        Compression level from 0 no compression to 9 maximum compression.
        Default: 5.
    :param int shuffle: One of:
        - Blosc.NOSHUFFLE (0): No shuffle
        - Blosc.SHUFFLE (1): byte-wise shuffle (default)
        - Blosc.BITSHUFFLE (2): bit-wise shuffle
    """

    NOSHUFFLE = 0
    """Flag to disable data shuffle pre-compression filter"""

    SHUFFLE = 1
    """Flag to enable byte-wise shuffle pre-compression filter"""

    BITSHUFFLE = 2
    """Flag to enable bit-wise shuffle pre-compression filter"""

    filter_id = BLOSC_ID

    __COMPRESSIONS = {
        'blosclz': 0,
        'lz4': 1,
        'lz4hc': 2,
        'snappy': 3,
        'zlib': 4,
        'zstd': 5,
    }

    def __init__(self, cname='lz4', clevel=5, shuffle=SHUFFLE):
        compression = self.__COMPRESSIONS[cname]
        clevel = int(clevel)
        assert 0 <= clevel <= 9
        assert shuffle in (self.NOSHUFFLE, self.SHUFFLE, self.BITSHUFFLE)
        self.filter_options = (0, 0, 0, 0, clevel, shuffle, compression)


class Bitshuffle(_FilterRefClass):
    """h5py.Group.create_dataset's compression and compression_opts arguments for using bitshuffle filter.

    :param int nelems:
        The number of elements per block.
        Default: 0 (for about 8kB per block).
    :param bool lz4:
        Whether to use LZ4_ID compression or not as part of the filter.
        Default: True
    """
    filter_id = BSHUF_ID

    def __init__(self, nelems=0, lz4=True):
        nelems = int(nelems)
        assert nelems % 8 == 0

        lz4_enabled = 2 if lz4 else 0
        self.filter_options = (nelems, lz4_enabled)


class LZ4(_FilterRefClass):
    """h5py.Group.create_dataset's compression and compression_opts arguments for using lz4 filter.

    :param int nelems:
        The number of bytes per block.
        Default: 0 (for 1GB per block).
    """
    filter_id = LZ4_ID

    def __init__(self, nbytes=0):
        nbytes = int(nbytes)
        assert 0 <= nbytes <= 0x7E000000
        self.filter_options = (nbytes,)


class FciDecomp(_FilterRefClass):
    """h5py.Group.create_dataset's compression and compression_opts arguments for using FciDecomp filter.
    """
    filter_id = FCIDECOMP_ID


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
        filename = _glob(_os.path.join(PLUGINS_PATH, 'libh5' + name + '*'))
        if len(filename):
            filename = filename[0]
        else:
            _logger.error("Cannot initialize filter %s: %d. File not found", name)
            continue
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
