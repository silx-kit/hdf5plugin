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

import ctypes
from glob import glob as _glob
import logging
import os as _os
import sys

import h5py


_logger = logging.getLogger(__name__)


# Check _version module to avoid importing from source
project = _os.path.basename(_os.path.dirname(_os.path.abspath(__file__)))

try:
    from ._version import __date__ as date  # noqa
    from ._version import version, version_info, hexversion, strictversion  # noqa
except ImportError:
    raise RuntimeError("Do NOT use %s from its sources: build it and use the built version" % project)


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

# Pre-defined compression_opts

BSHUF_LZ4_OPTS = (0, 2)
"""bitshuffle compression_opts with default block size and lz4 compression enabled"""


def _init_filters():
    """Initialise and register HDF5 filters with h5py

    Generator of tuples: (filename, library handle)
    """
    hdf5_version = h5py.h5.get_libversion()

    for name, filter_id in FILTERS.items():
        # Check if filter is already loaded (not on buggy HDF5 versions)
        if (1, 8, 20) <= hdf5_version < (1, 10) or hdf5_version >= (1, 10, 2):
            if h5py.h5z.filter_avail(filter_id):
                _logger.warning("%s filter already loaded, skip it.", name)
                continue

        # Load DLL
        filename = _glob(_os.path.join(PLUGINS_PATH, 'libh5' + name + '*'))[0]
        lib = ctypes.CDLL(filename)

        if sys.platform.startswith('win'):
            # Use register_filter function to register filter
            lib.register_filter.restype = ctypes.c_int
            retval = lib.register_filter()
        else:
            # Use init_filter function to initialize DLL and register filter
            lib.init_filter.argtypes = [ctypes.c_char_p]
            lib.init_filter.restype = ctypes.c_int
            if sys.version_info[0] >= 3:
                libname = bytes(h5py.h5z.__file__, encoding='utf-8')
            else:
                libname = h5py.h5z.__file__
            retval = lib.init_filter(libname)

        if retval < 0:
            _logger.error("Cannot initialize filter %s: %d", name, retval)
            continue

        yield filename, lib


_filters = dict(_init_filters())  # Store loaded filters
