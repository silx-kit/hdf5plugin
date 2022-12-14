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

import logging
import struct
from collections.abc import Mapping
import h5py

from ._config import build_config


logger = logging.getLogger(__name__)


# IDs of provided filters
BLOSC_ID = 32001
"""Blosc filter ID"""

BLOSC2_ID = 32026
"""Blosc 2 filter ID"""

BZIP2_ID = 307
"""Bzip2 filter ID"""

LZ4_ID = 32004
"""LZ4_ID filter ID"""

BSHUF_ID = 32008
"""Bitshuffle filter ID"""

ZFP_ID = 32013
"""ZFP filter ID"""

ZSTD_ID = 32015
"""Zstandard filter ID"""

SZ_ID = 32017
"""SZ filter ID"""

SZ3_ID = 32024
"""SZ3 filter ID"""

FCIDECOMP_ID = 32018
"""FCIDECOMP filter ID"""


try:
    _FilterRefClass = h5py.filters.FilterRefBase
except AttributeError:
    class _FilterRefClass(Mapping):
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


class Bitshuffle(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using bitshuffle filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'bitshuffle_with_lz4',
            data=numpy.arange(100),
            **hdf5plugin.Bitshuffle(nelems=0, lz4=True))
        f.close()

    :param int nelems:
        The number of elements per block.
        It needs to be divisible by eight (default is 0, about 8kB per block)
        Default: 0 (for about 8kB per block).
    :param str cname:
        `lz4` (default), `none`, `zstd`
    :param int clevel: Compression level, used only for `zstd` compression.
        Can be negative, and must be below or equal to 22 (maximum compression).
        Default: 3.
    """
    filter_name = "bshuf"
    filter_id = BSHUF_ID

    __COMPRESSIONS = {
        'none': 0,
        'lz4': 2,
        'zstd': 3,
    }

    def __init__(self, nelems=0, cname=None, clevel=3, lz4=None):
        nelems = int(nelems)
        assert nelems % 8 == 0
        assert clevel <= 22

        if lz4 is not None:
            if cname is not None:
                raise ValueError("Providing both cname and lz4 arguments is not supported")
            logger.warning(
                "Deprecation: hdf5plugin.Bitshuffle's lz4 argument is deprecated, "
                "use cname='lz4' or 'none' instead.")
            cname = 'lz4' if lz4 else 'none'

        if cname in (True, False):
            logger.warning(
                "Depreaction: hdf5plugin.Bitshuffle's boolean argument is deprecated, "
                "use cname='lz4' or 'none' instead.")
            cname = 'lz4' if cname else 'none'

        if cname is None:
            cname = 'lz4'
        if cname not in self.__COMPRESSIONS:
            raise ValueError(f"Unsupported compression: {cname}")

        if cname == 'zstd':
            self.filter_options = (nelems, self.__COMPRESSIONS[cname], clevel)
        else:
            self.filter_options = (nelems, self.__COMPRESSIONS[cname])


class Blosc(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using blosc filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'blosc_byte_shuffle_blosclz',
            data=numpy.arange(100),
            **hdf5plugin.Blosc(cname='blosclz', clevel=9, shuffle=hdf5plugin.Blosc.SHUFFLE))
        f.close()

    :param str cname:
        `blosclz`, `lz4` (default), `lz4hc`, `zlib`, `zstd`
        Optional: `snappy`, depending on compilation (requires C++11).
    :param int clevel:
        Compression level from 0 (no compression) to 9 (maximum compression).
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

    filter_name = "blosc"
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


class Blosc2(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using blosc2 filter.

    WARNING: This is a pre-release version of the HDF5 filter, only for testing purpose.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'blosc2_byte_shuffle_blosclz',
            data=numpy.arange(100),
            **hdf5plugin.Blosc2(cname='blosclz', clevel=9, shuffle=hdf5plugin.Blosc2.SHUFFLE))
        f.close()

    :param str cname:
        `blosclz` (default), `lz4`, `lz4hc`, `zlib`, `zstd`
    :param int clevel:
        Compression level from 0 (no compression) to 9 (maximum compression).
        Default: 5.
    :param int filters: One of:
        - Blosc2.NOFILTER (0): No pre-compression filter
        - Blosc2.SHUFFLE (1): Byte-wise shuffle (default)
        - Blosc2.BITSHUFFLE (2): Bit-wise shuffle
        - Blosc2.DELTA (3): Stores diff'ed blocks
        - Blosc3.TRUNC_PREC (4): Zeroes the least significant bits of the mantissa
    """

    NOFILTER = 0
    """Flag to disable pre-compression filter"""

    SHUFFLE = 1
    """Flag to enable byte-wise shuffle pre-compression filter"""

    BITSHUFFLE = 2
    """Flag to enable bit-wise shuffle pre-compression filter"""

    DELTA = 3
    """Flag to store blocks inside a chunk diff'ed with respect to first block in the chunk"""

    TRUNC_PREC = 4
    """Flag to zeroes the least significant bits of the mantissa of float32 and float64 types"""

    filter_id = BLOSC2_ID
    filter_name = "blosc2"

    __COMPRESSIONS = {
        'blosclz': 0,
        'lz4': 1,
        'lz4hc': 2,
        'zlib': 4,
        'zstd': 5,
    }

    def __init__(self, cname='blosclz', clevel=5, filters=SHUFFLE):
        compression = self.__COMPRESSIONS[cname]
        clevel = int(clevel)
        assert 0 <= clevel <= 9
        assert filters in (self.NOFILTER, self.SHUFFLE, self.BITSHUFFLE, self.DELTA, self.TRUNC_PREC)
        self.filter_options = (0, 0, 0, 0, clevel, filters, compression)


class BZip2(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using BZip2 filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'bzip2',
            data=numpy.arange(100),
            **hdf5plugin.BZip2(blocksize=5))
        f.close()

    :param int blocksize: Size of the blocks as a multiple of 100k
    """
    filter_name = "bzip2"
    filter_id = BZIP2_ID

    def __init__(self, blocksize=9) -> None:
        blocksize = int(blocksize)
        assert 1 <= blocksize <= 9
        self.filter_options = (blocksize,)


class FciDecomp(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using FciDecomp filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'fcidecomp',
            data=numpy.arange(100),
            **hdf5plugin.FciDecomp())
        f.close()
    """
    filter_name = "fcidecomp"
    filter_id = FCIDECOMP_ID

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if not build_config.cpp11:
            logger.error(
                "The FciDecomp filter is not available as hdf5plugin was not built with C++11.\n"
                "You may need to reinstall hdf5plugin with a recent version of pip, or rebuild it with a newer compiler.")


class LZ4(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using lz4 filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('lz4', data=numpy.arange(100),
            **hdf5plugin.LZ4(nbytes=0))
        f.close()

    :param int nbytes:
        The number of bytes per block.
        It needs to be in the range of 0 < nbytes < 2113929216 (1,9GB).
        Default: 0 (for 1GB per block).
    """
    filter_name = "lz4"
    filter_id = LZ4_ID

    def __init__(self, nbytes=0):
        nbytes = int(nbytes)
        assert 0 <= nbytes <= 0x7E000000
        self.filter_options = (nbytes,)


class Zfp(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using ZFP filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'zfp',
            data=numpy.random.random(100),
            **hdf5plugin.Zfp())
        f.close()

    This filter provides different modes:

    - **Fixed-rate** mode: To use, set the ``rate`` argument.
      For details, see `zfp fixed-rate mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-rate-mode>`_.

      .. code-block:: python

          f.create_dataset(
              'zfp_fixed_rate',
              data=numpy.random.random(100),
              **hdf5plugin.Zfp(rate=10.0))

    - **Fixed-precision** mode: To use, set the ``precision`` argument.
      For details, see `zfp fixed-precision mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-precision-mode>`_.

      .. code-block:: python

          f.create_dataset(
              'zfp_fixed_precision',
              data=numpy.random.random(100),
              **hdf5plugin.Zfp(precision=10))

    - **Fixed-accuracy** mode: To use, set the ``accuracy`` argument
      For details, see `zfp fixed-accuracy mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-accuracy-mode>`_.

      .. code-block:: python

          f.create_dataset(
              'zfp_fixed_accuracy',
              data=numpy.random.random(100),
              **hdf5plugin.Zfp(accuracy=0.001))

    - **Reversible** (i.e., lossless) mode: To use, set the ``reversible`` argument to True
      For details, see `zfp reversible mode <https://zfp.readthedocs.io/en/latest/modes.html#reversible-mode>`_.

      .. code-block:: python

          f.create_dataset(
              'zfp_reversible',
              data=numpy.random.random(100),
              **hdf5plugin.Zfp(reversible=True))

    - **Expert** mode: To use, set the ``minbits``, ``maxbits``, ``maxprec`` and ``minexp`` arguments.
      For details, see `zfp expert mode <https://zfp.readthedocs.io/en/latest/modes.html#expert-mode>`_.

      .. code-block:: python

          f.create_dataset(
              'zfp_expert',
              data=numpy.random.random(100),
              **hdf5plugin.Zfp(minbits=1, maxbits=16657, maxprec=64, minexp=-1074))

    :param float rate:
        Use fixed-rate mode and set the number of compressed bits per value.
    :param float precision:
        Use fixed-precision mode and set the number of uncompressed bits per value.
    :param float accuracy:
        Use fixed-accuracy mode and set the absolute error tolerance.
    :param bool reversible:
        If True, it uses the reversible (i.e., lossless) mode.
    :param int minbits: Minimum number of compressed bits used to represent a block.
    :param int maxbits: Maximum number of bits used to represent a block.
    :param int maxprec: Maximum number of bit planes encoded.
        It controls the relative error.
    :param int minexp: Smallest absolute bit plane number encoded.
        It controls the absolute error.
    """
    filter_name = "zfp"
    filter_id = ZFP_ID

    def __init__(self,
                 rate=None,
                 precision=None,
                 accuracy=None,
                 reversible=False,
                 minbits=None,
                 maxbits=None,
                 maxprec=None,
                 minexp=None):
        if rate is not None:
            rateHigh, rateLow = struct.unpack('II', struct.pack('d', float(rate)))
            self.filter_options = 1, 0, rateHigh, rateLow, 0, 0
            logger.info("ZFP mode 1 used. H5Z_ZFP_MODE_RATE")

        elif precision is not None:
            self.filter_options = 2, 0, int(precision), 0, 0, 0
            logger.info("ZFP mode 2 used. H5Z_ZFP_MODE_PRECISION")

        elif accuracy is not None:
            accuracyHigh, accuracyLow = struct.unpack(
                'II', struct.pack('d', float(accuracy)))
            self.filter_options = 3, 0, accuracyHigh, accuracyLow, 0, 0
            logger.info("ZFP mode 3 used. H5Z_ZFP_MODE_ACCURACY")

        elif reversible:
            self.filter_options = 5, 0, 0, 0, 0, 0
            logger.info("ZFP mode 5 used. H5Z_ZFP_MODE_REVERSIBLE")

        elif minbits is not None:
            minbits = int(minbits)
            maxbits = int(maxbits)
            maxprec = int(maxprec)
            minexp = struct.unpack('I', struct.pack('i', int(minexp)))[0]
            self.filter_options = 4, 0, minbits, maxbits, maxprec, minexp
            logger.info("ZFP mode 4 used. H5Z_ZFP_MODE_EXPERT")

        else:
            logger.info("ZFP default used")

        logger.info(f"filter options = {self.filter_options}")


class SZ(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using SZ filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'sz',
            data=numpy.random.random(100),
            **hdf5plugin.SZ())
        f.close()

    This filter provides different modes:

    - **Absolute** mode: To use, set the ``absolute`` argument.
      It ensures that the resulting values will be within the provided absolute tolerance.

      .. code-block:: python

          f.create_dataset(
              'sz_absolute',
              data=numpy.random.random(100),
              **hdf5plugin.SZ(absolute=0.1))

    - **Relative** mode: To use, set the ``relative`` argument.
      It ensures that the resulting values will be within the provided relative tolerance.
      The tolerance will be computed by multiplying the provided argument by the range of the data values.

      .. code-block:: python

          f.create_dataset(
              'sz_relative',
              data=numpy.random.random(100),
              **hdf5plugin.SZ(relative=0.01))

    - **Point-wise relative** mode: To use, set the ``pointwise_relative`` argument.
      It ensures that each grid point of the resulting values will be within the provided relative tolerance.

      .. code-block:: python

          f.create_dataset(
              'sz_pointwise_relative',
              data=numpy.random.random(100),
              **hdf5plugin.SZ(pointwise_relative=0.01))

    For more details about the compressor `SZ <https://szcompressor.org/>`_.
    """
    filter_name = "sz"
    filter_id = SZ_ID

    def __init__(self, absolute=None, relative=None, pointwise_relative=None):
        if (absolute, relative, pointwise_relative).count(None) < 2:
            raise TypeError("hdf5plugin.SZ() takes at most one not None argument")

        # Get SZ encoding options
        if absolute is not None:
            sz_mode = 0
        elif relative is not None:
            sz_mode = 1
        else:
            sz_mode = 10
            if pointwise_relative is None:
                pointwise_relative = 1e-5

        compression_opts = (
            sz_mode,
            *self.__pack_float64(absolute or 0.),
            *self.__pack_float64(relative or 0.),
            *self.__pack_float64(pointwise_relative or 0.),
            *self.__pack_float64(0.),  # psnr
        )

        logger.info(f"SZ mode {sz_mode} used.")
        logger.info(f"filter options {compression_opts}")

        self.filter_options = compression_opts

    @staticmethod
    def __pack_float64(error: float) -> tuple:
        packed = struct.pack('>d', error)  # Pack as big-endian IEEE 754 double
        high = struct.unpack('>I', packed[0:4])[0]  # Unpack most-significant bits as unsigned int
        low = struct.unpack('>I', packed[4:8])[0]  # Unpack least-significant bits as unsigned int
        return high, low


class SZ3(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using SZ3 filter.

    It can be passed as keyword arguments:

    - **Absolute** mode: To use, set the ``absolute`` argument.
      It ensures that the resulting values will be within the provided absolute tolerance.

      .. code-block:: python

          f.create_dataset(
              'sz3_absolute',
              data=numpy.random.random(100),
              **hdf5plugin.SZ3(absolute=0.1))

    For more details about the compressor, see `SZ3 <https://szcompressor.org/>`_.
    """
    filter_name = "sz3"
    filter_id = SZ3_ID

    def __init__(self, absolute=None, relative=None, norm2=None, peak_signal_to_noise_ratio=None):
        n_nones = (absolute, relative, norm2, peak_signal_to_noise_ratio).count(None)
        if n_nones < 3:
            raise TypeError("hdf5plugin.SZ3() takes at most one not None argument")
        elif n_nones == 4:
            absolute = 0.0001
            logger.warning(f"Defaulting to absolute={absolute}. This default might not be kept in future releases")

        # Get SZ3 encoding options: range [0, 5]
        if absolute is not None:
            sz_mode = 0
        elif relative is not None:
            sz_mode = 1
        elif norm2 is not None:
            sz_mode = 2
        elif peak_signal_to_noise_ratio is not None:
            sz_mode = 3
        if sz_mode not in [0, 2]:
            logger.warning("Only absolute and norm2 modes properly tested")

        compression_opts = (
            sz_mode,
            *self.__pack_float64(absolute or 0.),
            *self.__pack_float64(relative or 0.),
            *self.__pack_float64(norm2 or 0.),
            *self.__pack_float64(peak_signal_to_noise_ratio or 0.),
        )
        logger.info(f"SZ3 mode {sz_mode} used.")
        logger.info(f"filter options {compression_opts}")
        # 9 values needed
        if len(compression_opts) != 9:
            raise IndexError("Invalid number of arguments")

        self.filter_options = compression_opts

    @staticmethod
    def __pack_float64(error: float) -> tuple:
        packed = struct.pack('>d', error)  # Pack as big-endian IEEE 754 double
        high = struct.unpack('>I', packed[0:4])[0]  # Unpack most-significant bits as unsigned int
        low = struct.unpack('>I', packed[4:8])[0]  # Unpack least-significant bits as unsigned int
        return high, low


class Zstd(_FilterRefClass):
    """``h5py.Group.create_dataset``'s compression arguments for using FciDecomp filter.

    It can be passed as keyword arguments:

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'zstd',
            data=numpy.arange(100),
            **hdf5plugin.Zstd())
        f.close()

    :param int clevel: Compression level from 1 (lowest compression) to 22 (maximum compression).
        Ultra compression extends from 20 through 22. Default: 3.

    .. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset(
            'zstd',
            data=numpy.arange(100),
            **hdf5plugin.Zstd(clevel=22))
        f.close()
    """
    filter_name = "zstd"
    filter_id = ZSTD_ID

    def __init__(self, clevel=3):
        assert 1 <= clevel <= 22
        self.filter_options = (clevel,)


FILTER_CLASSES = Bitshuffle, Blosc, Blosc2, BZip2, FciDecomp, LZ4, SZ, SZ3, Zfp, Zstd


FILTERS = dict((cls.filter_name, cls.filter_id) for cls in FILTER_CLASSES)
"""Mapping of provided filter's name to their HDF5 filter ID."""
