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


def is_filter_available(name):
    """Returns whether filter is already registered or not.

    :param str name: Name of the filter (See `hdf5plugin.FILTERS`)
    :return: True if filter is registered, False if not and
        None if it cannot be checked (libhdf5 not supporting it)
    :rtype: Union[bool,None]
    """
    filter_id = FILTERS[name]

    hdf5_version = h5py.h5.get_libversion()
    if hdf5_version < (1, 8, 20) or (1, 10) <= hdf5_version < (1, 10, 2):
        return None # h5z.filter_avail not available
    return h5py.h5z.filter_avail(filter_id) > 0


registered_filters = {}
"""Store hdf5plugin registered filters as a mapping: name: (filename, ctypes.CDLL)"""


def register_filter(name):
    """Register a filter given its name

    Unregister the previously registered filter if any.

    :param str name: Name of the filter (See `hdf5plugin.FILTERS`)
    :return: True if successfully registered, False otherwise
    :rtype: bool
    """
    if name not in FILTERS:
        raise ValueError("Unknown filter name: %s" % name)

    if name not in build_config.embedded_filters:
        logger.debug("%s filter not available in this build of hdf5plugin.", name)
        return False

    # Unregister existing filter
    filter_id = FILTERS[name]
    is_avail = is_filter_available(name)
    # TODO h5py>=2.10
    if is_avail is True:
        if not h5py.h5z.unregister_filter(filter_id):
            logger.error("Failed to unregister filter %s (%d)" % (name, filter_id))
            return False
    elif is_avail is None:  # Cannot probe filter availability
        try:
            h5py.h5z.unregister_filter(filter_id)
        except RuntimeError:
            logger.debug("Filter %s (%d) not unregistered" % (name, filter_id))
            logger.debug(traceback.format_exc())
    registered_filters.pop(name, None)

    # Load DLL
    filename = glob.glob(os.path.join(
        PLUGIN_PATH, 'libh5' + name + '*' + build_config.filter_file_extension))
    if len(filename):
        filename = filename[0]
    else:
        logger.error("Cannot initialize filter %s: File not found", name)
        return False
    try:
        lib = ctypes.CDLL(filename)
    except OSError:
        logger.error("Failed to load filter %s: %s", name, filename)
        logger.error(traceback.format_exc())
        return False

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
        return False

    logger.debug("Registered filter: %s (%s)", name, filename)
    registered_filters[name] = filename, lib
    return True


HDF5PluginConfig = namedtuple(
    'HDF5PluginConfig',
    ('build_config', 'registered_filters'),
)


def get_config():
    """Provides information about build configuration and filters registered by hdf5plugin.
    """
    filters = {}
    for name in FILTERS:
        info = registered_filters.get(name)
        if info is not None:  # Registered by hdf5plugin
            if is_filter_available(name) in (True, None):
                filters[name] = info[0]
        elif is_filter_available(name) is True:  # Registered elsewhere
            filters[name] = "unknown"

    return HDF5PluginConfig(build_config, filters)


def register(filters=tuple(FILTERS.keys()), force=True):
    """Initialise and register `hdf5plugin` embedded filters given their names.

    Unregister corresponding previously registered filters if any.

    :param Union[str.Tuple[str]] filters:
        Filter name or sequence of filter names (See `hdf5plugin.FILTERS`).
    :param bool force:
        True to register the filter even if a corresponding one if already available.
        False to skip already available filters.
    :return: True if all filters were registered successfully, False otherwise.
    :rtype: bool
    """
    if isinstance(filters, str):
        filters = (filters,)

    status = True
    for filter_name in filters:
        if not force and is_filter_available(filter_name) is True:
            logger.info("%s filter already loaded, skip it.", filter_name)
            continue
        status = status and register_filter(filter_name)
    return status

register(force=False)
