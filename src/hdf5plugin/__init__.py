# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2024 European Synchrotron Radiation Facility
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

from ._version import version, version_info  # noqa

from ._filters import FILTERS  # noqa
from ._filters import BLOSC_ID, Blosc  # noqa
from ._filters import BLOSC2_ID, Blosc2  # noqa
from ._filters import BSHUF_ID, Bitshuffle  # noqa
from ._filters import BZIP2_ID, BZip2  # noqa
from ._filters import LZ4_ID, LZ4  # noqa
from ._filters import FCIDECOMP_ID, FciDecomp  # noqa
from ._filters import ZFP_ID, Zfp  # noqa
from ._filters import ZSTD_ID, Zstd  # noqa
from ._filters import SZ_ID, SZ  # noqa
from ._filters import SZ3_ID, SZ3  # noqa
from ._filters import SPERR_ID, Sperr  # noqa

from ._utils import get_config, get_filters, PLUGIN_PATH, register  # noqa

# Backward compatibility
PLUGINS_PATH = PLUGIN_PATH
