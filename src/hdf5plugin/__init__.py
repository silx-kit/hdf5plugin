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
"""This module provides compiled shared libraries for their use as HDF5 filters
under windows, MacOS and linux."""

from ._version import __date__ as date  # noqa
from ._version import version, version_info, hexversion, strictversion  # noqa

from ._filters import FILTERS  # noqa
from ._filters import BLOSC_ID, Blosc  # noqa
from ._filters import BSHUF_ID, Bitshuffle  # noqa
from ._filters import BZIP2_ID, BZip2  # noqa
from ._filters import LZ4_ID, LZ4  # noqa
from ._filters import FCIDECOMP_ID, FciDecomp  # noqa
from ._filters import ZFP_ID, Zfp  # noqa
from ._filters import ZSTD_ID, Zstd  # noqa
from ._filters import SZ_ID, SZ  # noqa

from ._utils import get_config, PLUGIN_PATH, register  # noqa

# Backward compatibility
from ._config import build_config as config  # noqa

PLUGINS_PATH = PLUGIN_PATH  # noqa
