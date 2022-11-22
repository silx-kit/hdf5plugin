# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2022 European Synchrotron Radiation Facility
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

import ctypes
import glob
import logging
import os
import sys
import traceback
from collections import namedtuple
import h5py

from ._filters import FILTERS
from ._config import build_config


logger = logging.getLogger(__name__)


PLUGIN_PATH = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'plugins'))
"""Directory where the provided HDF5 filter plugins are stored."""


def _init_filters():
    """Initialise and register HDF5 filters with h5py

    Generator of tuples: (filename, library handle)
    """
    hdf5_version = h5py.h5.get_libversion()

    for name, filter_id in FILTERS.items():
        # Skip filters that were not embedded
        if name not in build_config.embedded_filters:
            logger.debug("%s filter not available in this build of hdf5plugin.", name)
            continue

        # Check if filter is already loaded (not on buggy HDF5 versions)
        if (1, 8, 20) <= hdf5_version < (1, 10) or hdf5_version >= (1, 10, 2):
            if h5py.h5z.filter_avail(filter_id):
                logger.info("%s filter already loaded, skip it.", name)
                yield name, ("unknown", None)
                continue

        # Load DLL
        filename = glob.glob(os.path.join(
            PLUGIN_PATH, 'libh5' + name + '*' + build_config.filter_file_extension))
        if len(filename):
            filename = filename[0]
        else:
            logger.error("Cannot initialize filter %s: File not found", name)
            continue
        try:
            lib = ctypes.CDLL(filename)
        except OSError:
            logger.error("Failed to load filter %s: %s", name, filename)
            logger.error(traceback.format_exc())
            continue

        if sys.platform.startswith('win'):
            # Use register_filter function to register filter
            lib.register_filter.restype = ctypes.c_int
            retval = lib.register_filter()
        else:
            # Use init_filter function to initialize DLL and register filter
            lib.init_filter.argtypes = [ctypes.c_char_p]
            lib.init_filter.restype = ctypes.c_int
            retval = lib.init_filter(
                bytes(h5py.h5z.__file__, encoding='utf-8'))

        if retval < 0:
            logger.error("Cannot initialize filter %s: %d", name, retval)
            continue

        logger.debug("Registered filter: %s (%s)", name, filename)
        yield name, (filename, lib)


_filters = dict(_init_filters())  # Store loaded filters


def get_config():
    """Provides information about build configuration and filters registered by hdf5plugin.
    """
    HDF5PluginConfig = namedtuple(
        'HDF5PluginConfig',
        ('build_config', 'registered_filters'),
    )
    return HDF5PluginConfig(
        build_config=build_config,
        registered_filters=dict((name, filename) for name, (filename, lib) in _filters.items()),
    )
