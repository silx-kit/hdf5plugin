# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2023 European Synchrotron Radiation Facility
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

from ._filters import FILTER_CLASSES, FILTERS
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
        return None  # h5z.filter_avail not available
    return h5py.h5z.filter_avail(filter_id) > 0


def H5Zregister_ctypes(filter_struct_p):
    """Register a new filter with libHDF5 using ctypes wrapping.

    :param ctypes.c_void_p filter_struct_p: Pointer to filter definition struct
    :return: A non-negative value if successful, else a negative value
    :rtype: int
    """
    if sys.platform.startswith("win"):
        libhdf5 = ctypes.cdll.LoadLibrary("hdf5")
    else:
        libhdf5 = ctypes.CDLL(h5py.h5z.__file__)
    libhdf5.H5Zregister.argtypes = [ctypes.c_void_p]
    libhdf5.H5Zregister.restype = ctypes.c_int
    return libhdf5.H5Zregister(filter_struct_p)


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
        raise ValueError(f"Unknown filter name: {name}")

    if name not in build_config.embedded_filters:
        logger.debug(f"{name} filter not available in this build of hdf5plugin.")
        return False

    # Unregister existing filter
    filter_id = FILTERS[name]
    is_avail = is_filter_available(name)
    if is_avail is True:
        if not h5py.h5z.unregister_filter(filter_id):
            logger.error(f"Failed to unregister filter {name} ({filter_id})")
            return False
    if is_avail is None:  # Cannot probe filter availability
        try:
            h5py.h5z.unregister_filter(filter_id)
        except RuntimeError:
            logger.debug(f"Filter {name} ({filter_id}) not unregistered")
            logger.debug(traceback.format_exc())
    registered_filters.pop(name, None)

    # Load DLL
    filenames = glob.glob(os.path.join(
        PLUGIN_PATH, f"libh5{name}*{build_config.filter_file_extension}"))
    if len(filenames):
        if name == 'blosc':  # Handle name prefix conflict with blosc2
            for filename in filenames:
                if not os.path.basename(filename).startswith('libh5blosc2'):
                    break  # That's the blosc(1) filename
            else:
                logger.error("Cannot initialize filter %s: File not found", name)
                return False
        elif name == 'sz':  # Handle name prefix conflict with sz3
            for filename in filenames:
                if not os.path.basename(filename).startswith('libh5sz3'):
                    break  # That's the sz filename
            else:
                logger.error("Cannot initialize filter %s: File not found", name)
                return False
        else:
            filename = filenames[0]
    else:
        logger.error(f"Cannot initialize filter {name}: File not found")
        return False
    try:
        lib = ctypes.CDLL(filename)
    except OSError:
        logger.error(f"Failed to load filter {name}: {filename}")
        logger.error(traceback.format_exc())
        return False

    if not sys.platform.startswith('win'):
        # Use init_filter function to initialize DLL
        try:
            init_filter = lib.init_filter
        except AttributeError:
            logger.debug(f"init_filter not found for filter {name}: Init phase skipped.")
        else:
            init_filter.argtypes = [ctypes.c_char_p]
            init_filter.restype = ctypes.c_int

            retval = init_filter(bytes(h5py.h5z.__file__, encoding='utf-8'))
            if retval < 0:
                logger.error(f"Cannot initialize filter {name}: {retval}")
                return False

    # Register through H5Zregister
    lib.H5PLget_plugin_info.restype = ctypes.c_void_p
    if h5py.version.version_tuple[:3] > (3, 8, 0):
        try:
            h5py.h5z.register_filter(lib.H5PLget_plugin_info())
        except:  # noqa: E722
            logger.error(f"Cannot register filter {name}: {retval}")
            logger.error(traceback.format_exc())
            return False
    else:
        retval = H5Zregister_ctypes(lib.H5PLget_plugin_info())
        if retval < 0:
            logger.error(f"Cannot register filter {name}: {retval}")
            return False

    logger.debug(f"Registered filter: {name} ({filename})")
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


def get_filters(filters=tuple(FILTERS.keys())):
    """Returns selected filter classes.

    By default it returns all filter classes.

    :param Union[str,int,Tuple[Union[str,int]] filters:
        Filter name or ID or sequence of filter names or IDs (default: all filters).
        It also supports the value `"registered"` which selects
        currently available filters.
    :return: Tuple of filter classes
    """
    if filters == "registered":
        filters = tuple(get_config().registered_filters.keys())
    if isinstance(filters, (str, int)):
        filters = (filters,)

    filter_classes = []
    for name_or_id in filters:
        if not isinstance(name_or_id, (str, int)):
            raise ValueError(f"Expected int or str, not {type(name_or_id)}")

        for cls in FILTER_CLASSES:
            if (
                isinstance(name_or_id, str) and cls.filter_name == name_or_id.lower()
            ) or (isinstance(name_or_id, int) and cls.filter_id == name_or_id):
                filter_classes.append(cls)
                break
        else:
            raise ValueError(f"Unknown filter: {name_or_id}")

    return tuple(filter_classes)


def register(filters=tuple(FILTERS.keys()), force=True):
    """Initialise and register `hdf5plugin` embedded filters given their names or IDs.

    :param Union[str,int,Tuple[Union[str,int]] filters:
        Filter name or ID or sequence of filter names or IDs.
    :param bool force:
        True to register the filter even if a corresponding one if already available.
        False to skip already available filters.
    :return: True if all filters were registered successfully, False otherwise.
    :rtype: bool
    """
    filter_classes = get_filters(filters)

    status = True
    for filter_class in filter_classes:
        filter_name = filter_class.filter_name
        if not force and is_filter_available(filter_name) is True:
            logger.info(f"{filter_name} filter already loaded, skip it.")
            continue
        status = register_filter(filter_name) and status
    return status


register(force=False)
