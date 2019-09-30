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
"""This module provides compiled shared libraries for their use as HDF5 plugins
under windows, MacOS and linux."""

__authors__ = ["V.A. Sole", "H. Payno", "T. Vincent"]
__license__ = "MIT"
__date__ = "30/09/2019"

import ctypes
from glob import glob as _glob
import logging
import os
import sys


_logger = logging.getLogger(__name__)


if sys.platform.startswith("win"):
    if sys.version_info >= (3, 5):
        compiler = "VS2015"
    elif sys.version.startswith("2.7"):
        compiler = "VS2008"
    else:
        compiler = None
    if sys.maxsize > 2**32:
        # 64 bit
        arch = "x64"
    else:
        # 32 bit
        arch = "x86"
    path_separator = ";"
else:
    compiler = None

if compiler is not None:
    if ("h5py" in sys.modules) or ("PyTables" in sys.modules):
        raise ImportError("You should import hdf5plugin before importing h5py or PyTables")
    current_path = os.getenv("HDF5_PLUGIN_PATH", None)
    plugin_path = os.path.join(os.path.dirname(__file__),
                                       compiler)
    if arch:
        plugin_path = os.path.join(plugin_path,
                                       arch)
        
    if current_path is None:
        os.environ["HDF5_PLUGIN_PATH"]  = plugin_path
    else:
        os.environ["HDF5_PLUGIN_PATH"] = current_path + path_separator + plugin_path


PLUGINS_PATH = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'plugins'))
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


def _init_plugins():
    """Initialise and register HDF5 filter plugins with h5py

    Generator of tuples: (filename, library handle)
    """
    import h5py

    hdf5_version = h5py.h5.get_libversion()

    for name, filter_id in FILTERS.items():
        # Check if filter is already loaded (not on buggy HDF5 versions)
        if (1, 8, 20) <= hdf5_version < (1, 10) or hdf5_version >= (1, 10, 2):
            if h5py.h5z.filter_avail(filter_id):
                _logger.warning("%s filter already loaded, skip it.", name)
                continue

        # Load DLL
        filename = _glob(os.path.join(PLUGINS_PATH, 'libh5' + name + '*'))[0]
        lib = ctypes.CDLL(filename)

        if hasattr(lib, 'init_plugin'):
            # There is a init_plugin function: initialize DLL
            lib.init_plugin.restype = None
            lib.init_plugin(bytes(h5py.h5z.__file__, encoding='utf-8'))

        # Register plugin with h5py's libhdf5
        # TODO check plugin_type = lib.H5PLget_plugin_type()

        # Get plugin info struct
        lib.H5PLget_plugin_info.restype = ctypes.c_void_p
        plugin_info = lib.H5PLget_plugin_info()

        # Register plugin to h5py
        # TODO h5py.h5z.register(plugin_info)
        h5zlib = ctypes.CDLL(h5py.h5z.__file__)
        h5zlib.H5Zregister.argtypes = [ctypes.c_void_p]
        h5zlib.H5Zregister.restype = ctypes.c_int
        if h5zlib.H5Zregister(plugin_info) != 0:
            _logger.error('cannot register plugin: %s', filename)
        else:
            yield filename, lib


_plugins = dict(_init_plugins())  # Store loaded plugins
