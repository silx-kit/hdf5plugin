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

It works under Windows, MacOS and Linux.
"""

from . import _version
from ._filters import FILTERS  # noqa
from ._filters import (  # noqa
    BLOSC2_ID,
    BLOSC_ID,
    BSHUF_ID,
    BZIP2_ID,
    FCIDECOMP_ID,
    LZ4,
    LZ4_ID,
    SPERR_ID,
    SZ,
    SZ3,
    SZ3_ID,
    SZ_ID,
    ZFP_ID,
    ZSTD_ID,
    Bitshuffle,
    Blosc,
    Blosc2,
    BZip2,
    FciDecomp,
    Sperr,
    Zfp,
    Zstd,
)
from ._utils import PLUGIN_PATH, get_config, get_filters, register  # noqa
from ._version import version  # noqa

# Backward compatibility
PLUGINS_PATH = PLUGIN_PATH


def __getattr__(name: str) -> _version.VersionInfo:
    if name == "version_info":
        return _version.get_version_info()
    raise AttributeError(f"module '{__name__}' has no attribute '{name}'")
