# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016 European Synchrotron Radiation Facility
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
under windows."""

__authors__ = ["V.A. Sole"]
__license__ = "MIT"
__date__ = "06/07/2016"

import os
import sys

if sys.platform.startswith("win"):
    arch = None
    if ("h5py" in sys.modules) or ("PyTables" in sys.modules):
        raise ImportError("You should import hdf5plugin before importing h5py or PyTables")
    if sys.version.startswith("3.5"):
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
    current_path = os.getenv("HDF5_PLUGIN_PATH", None)
    if arch is not None:
        plugin_path = os.path.join(os.path.dirname(__file__),
                                       compiler,
                                       arch)
        os.environ["HDF5_PLUGIN_PATH"]  = plugin_path
        if current_path is not None:
            os.environ["HDF5_PLUGIN_PATH"] += ";" + current_path
                          
        
