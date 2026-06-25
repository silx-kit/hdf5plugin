# /*##########################################################################
#
# Copyright (c) 2019-2024 European Synchrotron Radiation Facility
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
"""Provides tests"""

from __future__ import annotations

import importlib.util
import io
import os
import shutil
import tempfile
import unittest
from typing import Any, cast

import numpy
from packaging.version import parse as parse_version

import h5py
import hdf5plugin

try:
    import blosc2
except ImportError:
    blosc2 = None

from numpy.typing import DTypeLike

from hdf5plugin import _filters

BUILD_CONFIG = hdf5plugin.get_config().build_config


def should_test(filter_name: str) -> bool:
    """Returns True if the given filter should be tested"""
    filter_id = hdf5plugin.FILTERS[filter_name]
    return filter_name in BUILD_CONFIG.embedded_filters or h5py.h5z.filter_avail(
        filter_id
    )


compression_name_to_class = {
    "blosc": hdf5plugin.Blosc,
    "blosc2": hdf5plugin.Blosc2,
    "bshuf": hdf5plugin.Bitshuffle,
    "bzip2": hdf5plugin.BZip2,
    "lz4": hdf5plugin.LZ4,
    "fcidecomp": hdf5plugin.FciDecomp,
    "sperr": hdf5plugin.Sperr,
    "sz": hdf5plugin.SZ,
    "sz3": hdf5plugin.SZ3,
    "zfp": hdf5plugin.Zfp,
    "zstd": hdf5plugin.Zstd,
}


class BaseTestHDF5PluginRW(unittest.TestCase):
    """Base class for testing write/read HDF5 dataset with the plugins"""

    _data_natoms = 1000
    _data_shape = (100, 10)

    @classmethod
    def setUpClass(cls):
        cls.tempdir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tempdir)

    def _test(
        self,
        filter_name: str,
        dtype: DTypeLike = numpy.int32,
        lossless: bool = True,
        compressed: str | bool = True,
        options: dict[str, Any] = None,
    ):
        """Run test for a particular filter

        :param filter_name: The name of the filter to use
        :param options: create_dataset's compression_opts argument
        :return: The tuple describing the filter
        """
        data = numpy.ones((self._data_natoms,), dtype=dtype).reshape(self._data_shape)
        filename = os.path.join(self.tempdir, "test_" + filter_name + ".h5")
        compression_class = compression_name_to_class[filter_name]

        # Write
        f = h5py.File(filename, "w")
        f.create_dataset(
            "data",
            data=data,
            chunks=data.shape,
            compression=compression_class(**(options or {})),
        )
        f.close()

        # Read
        with h5py.File(filename, "r") as f:
            saved = f["data"][()]
            plist = f["data"].id.get_create_plist()
            filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]

            # Read chunk raw (compressed) data
            chunk = f["data"].id.read_direct_chunk((0,) * data.ndim)[1]

            if compressed is True:  # Check if chunk is actually compressed
                self.assertLess(len(chunk), data.nbytes)
            elif compressed is False:
                self.assertEqual(len(chunk), data.nbytes)
            else:
                assert compressed == "nocheck"

        if lossless:
            self.assertTrue(numpy.array_equal(saved, data))
        else:
            self.assertTrue(numpy.allclose(saved, data))
        self.assertEqual(saved.dtype, data.dtype)

        self.assertEqual(len(filters), 1)
        self.assertEqual(filters[0][0], hdf5plugin.FILTERS[filter_name])

        os.remove(filename)
        return filters[0]


class TestHDF5PluginRW(BaseTestHDF5PluginRW):
    """Test write/read a HDF5 file with the plugins"""

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testDepreactedBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test("bshuf")  # Default options

        # Specify options
        for lz4 in (False, True):
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(lz4=lz4, dtype=dtype, nelems=nelems):
                        filter_ = self._test(
                            "bshuf",
                            dtype,
                            compressed=lz4,
                            options={"nelems": nelems, "lz4": lz4},
                        )
                        self.assertEqual(filter_[2][3:], (nelems, 2 if lz4 else 0))

    def _get_bitshuffle_version(self) -> tuple[int, int]:
        filename = os.path.join(self.tempdir, "get_bitshuffle_version.h5")
        with h5py.File(filename, "w", driver="core", backing_store=False) as h5f:
            h5f.create_dataset(
                "data", data=numpy.arange(10), compression=hdf5plugin.Bitshuffle()
            )
            plist = h5f["data"].id.get_create_plist()
            assert plist.get_nfilters() == 1
            filter_ = plist.get_filter(0)
            assert filter_[0] == hdf5plugin.BSHUF_ID
            return tuple(filter_[2][:2])

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test("bshuf")  # Default options

        compressions = {  # Compressor name: Compressor ID
            "none": 0,
            "lz4": 2,
        }
        if self._get_bitshuffle_version() >= (0, 4):
            compressions["zstd"] = 3

        # Specify options
        for cname, compression_id in compressions.items():
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(cname=cname, dtype=dtype, nelems=nelems):
                        filter_ = self._test(
                            "bshuf",
                            dtype,
                            compressed=cname != "none",
                            options={
                                "nelems": nelems,
                                "cname": cname,
                            },
                        )
                        self.assertEqual(filter_[2][3:5], (nelems, compression_id))

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testBitshuffleZstdCLevel(self):
        """Write/read test with bitshuffle+zstd with different compression levels"""
        for clevel in (0, 3, 22):
            with self.subTest(clevel=clevel):
                filter_ = self._test(
                    "bshuf",
                    numpy.int32,
                    compressed=True,
                    options={"cname": "zstd", "clevel": clevel},
                )
                self.assertEqual(filter_[2][3:6], (0, 3, clevel))

    @unittest.skipUnless(should_test("blosc"), "Blosc filter not available")
    def testBlosc(self):
        """Write/read test with blosc filter plugin"""
        self._test("blosc")  # Default options

        # Specify options
        shuffles = (
            hdf5plugin.Blosc.NOSHUFFLE,
            hdf5plugin.Blosc.SHUFFLE,
            hdf5plugin.Blosc.BITSHUFFLE,
        )
        compress = "blosclz", "lz4", "lz4hc", "snappy", "zlib", "zstd"
        for compression_id, cname in enumerate(compress):
            for shuffle in shuffles:
                for clevel in range(10):
                    with self.subTest(
                        compression=cname, shuffle=shuffle, clevel=clevel
                    ):
                        if cname == "snappy" and not BUILD_CONFIG.cpp11:
                            self.skipTest("snappy unavailable without C++11")
                        filter_ = self._test(
                            "blosc",
                            compressed=clevel != 0,  # No compression for clevel=0
                            options={
                                "cname": cname,
                                "clevel": clevel,
                                "shuffle": shuffle,
                            },
                        )
                        self.assertEqual(
                            filter_[2][4:], (clevel, shuffle, compression_id)
                        )

    @unittest.skipUnless(should_test("blosc2"), "Blosc2 filter not available")
    def testBlosc2(self):
        """Write/read test with blosc2 filter plugin"""
        self._test("blosc2")  # Default options

        # Specify options
        tested_filters = (
            hdf5plugin.Blosc2.NOFILTER,
            hdf5plugin.Blosc2.SHUFFLE,
            hdf5plugin.Blosc2.BITSHUFFLE,
        )
        compress = "blosclz", "lz4", "lz4hc", "unused", "zlib", "zstd"
        for compression_id, cname in enumerate(compress):
            if cname == "unused":
                continue
            for filters in tested_filters:
                for clevel in range(10):
                    with self.subTest(
                        compression=cname, filters=filters, clevel=clevel
                    ):
                        filter_ = self._test(
                            "blosc2",
                            compressed=(
                                "nocheck" if clevel == 0 else True
                            ),  # For clevel=0, chunks are larger
                            options={
                                "cname": cname,
                                "clevel": clevel,
                                "filters": filters,
                            },
                        )
                        filter_params: tuple[int | str, ...] = (
                            clevel,
                            filters,
                            compression_id,
                        )
                        if len(self._data_shape) >= 2:
                            # Chunk shape passed to filter code
                            filter_params += (len(self._data_shape),) + self._data_shape
                        self.assertEqual(filter_[2][4:], filter_params)

    @unittest.skipUnless(should_test("bzip2"), "BZip2 filter not available")
    def testBZip2(self):
        """Write/read test with BZip2 filter plugin"""
        self._test("bzip2")  # Default options

        # Specify options
        for blocksize in range(1, 10):
            with self.subTest(blocksize=blocksize):
                filter_ = self._test("bzip2", options={"blocksize": blocksize})
                self.assertEqual(filter_[2][0], blocksize)

    @unittest.skipUnless(should_test("lz4"), "LZ4 filter not available")
    def testLZ4(self):
        """Write/read test with lz4 filter plugin"""
        self._test("lz4")

        # Specify options
        filter_ = self._test("lz4", options={"nbytes": 1024})
        self.assertEqual(filter_[2], (1024,))

    @unittest.skipUnless(should_test("fcidecomp"), "FCIDECOMP filter not available")
    def testFciDecomp(self):
        """Write/read test with fcidecomp filter plugin"""
        # Test with supported datatypes
        for dtype in (numpy.uint8, numpy.uint16, numpy.int8, numpy.int16):
            with self.subTest(dtype=dtype):
                self._test("fcidecomp", dtype=dtype)

    @unittest.skipUnless(should_test("sperr"), "Sperr filter not available")
    def testSperr(self):
        """Write/read test with Sperr filter plugin"""
        tests: list[dict[str, int | float | bool]] = [
            {"rate": 16},
            {"rate": 16, "swap": True},
            {
                "rate": 16,
                "swap": True,
                "missing_value_mode": hdf5plugin.Sperr.MISSING_NAN,
            },
            {
                "rate": 16,
                "swap": True,
                "missing_value_mode": hdf5plugin.Sperr.MISSING_1E35,
            },
            {"peak_signal_to_noise_ratio": 1e-4},
            {"peak_signal_to_noise_ratio": 1e-4, "swap": True},
            {"absolute": 1e-4},
            {"absolute": 1e-4, "swap": True},
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test("sperr", dtype=dtype, lossless=False, options=options)

    @unittest.skipUnless(should_test("sz"), "SZ filter not available")
    def testSZ(self):
        """Write/read test with SZ filter plugin"""
        # TODO: Options mission
        tests = [
            {"absolute": 0.0001},
            {"relative": 0.01},
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test("sz", dtype=dtype, lossless=False, options=options)

    @unittest.skipUnless(should_test("sz3"), "SZ3 filter not available")
    def testSZ3(self):
        """Write/read test with SZ3 filter plugin"""
        # TODO: Options mission
        tests = [
            {"absolute": 0.001},
            # {'relative': 0.0001},
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test("sz3", dtype=dtype, lossless=False, options=options)

    @unittest.skipUnless(should_test("zfp"), "ZFP filter not available")
    def testZfp(self):
        """Write/read test with zfp filter plugin"""
        tests: list[dict[str, int | float | bool]] = [
            {},  # Default config
            {"rate": 10.0},  # Fixed-rate
            {"precision": 10},  # Fixed-precision
            {"accuracy": 1e-8},  # Fixed-accuracy
            {"reversible": True},  # Reversible
            # Expert: with default parameters
            {
                "minbits": 1,
                "maxbits": 16657,
                "maxprec": 64,
                "minexp": -1074,
            },
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test(
                        "zfp",
                        dtype=dtype,
                        lossless=bool(options.get("reversible", False)),
                        options=options,
                    )

        self._test("zfp", dtype=numpy.int32, options={"reversible": True})

    @unittest.skipUnless(should_test("zstd"), "Zstd filter not available")
    def testZstd(self):
        """Write/read test with Zstd filter plugin"""
        self._test("zstd")
        tests = [{"clevel": 3}, {"clevel": 22}]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test("zstd", dtype=dtype, options=options)


class TestStrings(unittest.TestCase):
    """Test strings compression"""

    @classmethod
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        N = 100
        self.string_arrays = [
            # Note: h5py does not support dtype="U"
            numpy.array(["test", "strings", "ascii"] * N, dtype="S"),
            numpy.array([b"test", b"strings", b"binary"] * N, dtype="O"),
        ]
        has_h5py_314 = parse_version(h5py.__version__) >= parse_version("3.14")
        has_numpy_2 = parse_version(numpy.__version__) >= parse_version("2.0")
        if has_h5py_314 and has_numpy_2:
            self.string_arrays.append(
                numpy.array(["test", "strings", "Crème brûlée"] * N, dtype="T")
            )

    @classmethod
    def tearDown(self):
        self.tempdir.cleanup()

    def _test_strings(
        self,
        filter_name: str,
        options: dict[str, Any] | None = None,
    ) -> None:
        """Test string compression for a particular filter

        :param filter_name: The name of the filter to use
        """
        filename = os.path.join(self.tempdir.name, f"{filter_name}.h5")
        compression_class = compression_name_to_class[filter_name]

        for data in self.string_arrays:
            with self.subTest(name=data.dtype.kind):
                ds_name = f"data{data.dtype.kind}"
                # Write
                with h5py.File(filename, "w") as f:
                    f.create_dataset(
                        ds_name,
                        data=data,
                        chunks=data.shape,
                        compression=compression_class(**(options or {})),
                    )

                # Read
                with h5py.File(filename, "r") as f:
                    if data.dtype.kind == "T":
                        # Use h5py accessor. Note that this is very different from
                        # f[ds_name][()].astype("T")
                        saved = f[ds_name].astype("T")[()]
                    else:
                        saved = f[ds_name][()]

                    plist = f[ds_name].id.get_create_plist()
                    filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]

                    # Read chunk raw (compressed) data
                    chunk = f[ds_name].id.read_direct_chunk((0,))[1]

                    # Check if chunk is actually compressed
                    self.assertLess(len(chunk), data.nbytes)

                self.assertTrue(numpy.array_equal(saved, data))
                self.assertEqual(saved.dtype, data.dtype)

                self.assertEqual(len(filters), 1)
                self.assertEqual(filters[0][0], hdf5plugin.FILTERS[filter_name])

    @unittest.skip(reason="segfault (#364)")
    @unittest.skipUnless(should_test("blosc"), "Blosc filter not available")
    def testStringsBlosc(self):
        """Strings write/read test with blosc filter plugin"""
        self._test_strings("blosc")  # Default options

    @unittest.skip(reason="segfault (#364)")
    @unittest.skipUnless(should_test("blosc2"), "Blosc filter not available")
    def testStringsBlosc2(self):
        """Strings write/read test with blosc2 filter plugin"""
        self._test_strings("blosc2")

    @unittest.skipUnless(should_test("bzip2"), "BZip2 filter not available")
    def testStringsBZip2(self):
        """Strings write/read test with BZip2 filter plugin"""
        self._test_strings("bzip2")

    @unittest.skipUnless(should_test("lz4"), "LZ4 filter not available")
    def testStringsLZ4(self):
        """Strings write/read test with LZ4 filter plugin"""
        self._test_strings("lz4")

    @unittest.skipUnless(should_test("zstd"), "Zstd filter not available")
    def testStringsZstd(self):
        """Strings write/read test with Zstd filter plugin"""
        self._test_strings("zstd")


class TestFromFilterOptionsMethods(unittest.TestCase):
    """Test _from_filter_options methods"""

    def testBitshuffle(self):
        for filter_options, expected_options in (
            # (_, _, _, nelems, compression_id, clevel)
            ((), (0, 0)),  # Default: no compression
            ((0, 2, 4, 256), (256, 0)),  # custom nelems
            ((0, 2, 4, 0, 2), (0, 2)),  # LZ4
            ((0, 2, 4, 0, 3), (0, 3, 3)),  # Zstd with default clevel
            ((0, 2, 4, 0, 3, 5), (0, 3, 5)),  # Zstd with custom clevel
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Bitshuffle._from_filter_options(
                    filter_options
                )
                self.assertEqual(compression_filter.filter_options, expected_options)

    def testBlosc(self):
        for filter_options, expected_options in (
            # (_, _, _, _, clevel, shuffle, compression_id)
            ((), (0, 0, 0, 0, 5, 1, 0)),  # Default: no compression
            ((2, 2, 4, 40000, 3), (0, 0, 0, 0, 3, 1, 0)),  # custom clevel
            (
                (2, 2, 4, 40000, 3, 2),
                (0, 0, 0, 0, 3, 2, 0),
            ),  # custom clevel and shuffle
            ((2, 2, 4, 40000, 8, 2, 1), (0, 0, 0, 0, 8, 2, 1)),  # all custom
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Blosc._from_filter_options(
                    filter_options
                )
                self.assertEqual(compression_filter.filter_options, expected_options)

    def testBlosc2(self):
        for filter_options, expected_options in (
            # (_, _, _, _, clevel, filters, compression_id)
            ((), (0, 0, 0, 0, 5, 1, 0)),  # Default: no compression
            ((2, 2, 4, 40000, 3), (0, 0, 0, 0, 3, 1, 0)),  # custom clevel
            (
                (2, 2, 4, 40000, 3, 2),
                (0, 0, 0, 0, 3, 2, 0),
            ),  # custom clevel and filters
            ((2, 2, 4, 40000, 8, 2, 1), (0, 0, 0, 0, 8, 2, 1)),  # all custom
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Blosc2._from_filter_options(
                    filter_options
                )
                self.assertEqual(compression_filter.filter_options, expected_options)

    def testBZip2(self):
        for filter_options, expected_options in (
            # (blocksize,)
            ((), (9,)),
            ((5,), (5,)),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.BZip2._from_filter_options(
                    filter_options
                )
                self.assertEqual(compression_filter.filter_options, expected_options)

    def testFciDecomp(self):
        compression_filter = hdf5plugin.FciDecomp._from_filter_options((1, 2, 3))
        self.assertEqual(compression_filter.filter_options, ())

    def testLZ4(self):
        for filter_options, expected_options in (
            # (nbytes,)
            ((), (0,)),
            ((1024,), (1024,)),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.LZ4._from_filter_options(filter_options)
                self.assertEqual(compression_filter.filter_options, expected_options)

    def testSperr(self):
        for filter_options, expected_filter in (
            ((1043, 269484032, 128, 0, 0), hdf5plugin.Sperr()),
            (
                (1107, 2418016256, 256, 0, 0),
                hdf5plugin.Sperr(rate=32, swap=True, missing_value_mode=1),
            ),
            ((1043, 940177214, 256, 0, 0), hdf5plugin.Sperr(absolute=1e-3)),
            (
                (1171, 537001984, 256, 0, 0),
                hdf5plugin.Sperr(peak_signal_to_noise_ratio=2.0, missing_value_mode=2),
            ),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Sperr._from_filter_options(
                    filter_options
                )
                self.assertEqual(
                    compression_filter.filter_options, expected_filter.filter_options
                )

    def testSZ(self):
        for filter_options, expected_filter in (
            (
                (1, 0, 0, 256, 10, 0, 0, 0, 0, 1055193269, 2296604913, 0, 0),
                hdf5plugin.SZ(),
            ),
            (
                (1, 0, 0, 256, 0, 1062232653, 3539053052, 0, 0, 0, 0, 0, 0),
                hdf5plugin.SZ(absolute=1e-3),
            ),
            (
                (1, 0, 0, 256, 1, 0, 0, 1062232653, 3539053052, 0, 0, 0, 0),
                hdf5plugin.SZ(relative=1e-3),
            ),
            (
                (1, 0, 0, 256, 10, 0, 0, 0, 0, 1062232653, 3539053052, 0, 0),
                hdf5plugin.SZ(pointwise_relative=1e-3),
            ),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.SZ._from_filter_options(filter_options)
                self.assertEqual(
                    compression_filter.filter_options, expected_filter.filter_options
                )

    def testSZ3(self):
        for filter_options, expected_filter in (
            (
                (1, 0, 0, 256, 0, 1058682594, 3944497965, 0, 0, 0, 0, 0, 0),
                hdf5plugin.SZ3(),
            ),
            (
                (1, 0, 0, 256, 0, 1051772663, 2696277389, 0, 0, 0, 0, 0, 0),
                hdf5plugin.SZ3(absolute=1e-6),
            ),
            (
                (1, 0, 0, 256, 1, 0, 0, 1062232653, 3539053052, 0, 0, 0, 0),
                hdf5plugin.SZ3(relative=1e-3),
            ),
            (
                (1, 0, 0, 256, 2, 0, 0, 0, 0, 1062232653, 3539053052, 0, 0),
                hdf5plugin.SZ3(norm2=1e-3),
            ),
            (
                (1, 0, 0, 256, 3, 0, 0, 0, 0, 0, 0, 1062232653, 3539053052),
                hdf5plugin.SZ3(peak_signal_to_noise_ratio=1e-3),
            ),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.SZ3._from_filter_options(filter_options)
                self.assertEqual(
                    compression_filter.filter_options, expected_filter.filter_options
                )

    def testZfp(self):
        for filter_options, expected_filter in (
            (
                (269504785, 91252346, 4026532854, 2167406593),
                hdf5plugin.Zfp(precision=20),
            ),
            (
                (269504785, 91252346, 4026532854, 3404726273),
                hdf5plugin.Zfp(accuracy=2**-4),
            ),
            (
                (269504785, 91252346, 4026532854, 2281701377),
                hdf5plugin.Zfp(reversible=True),
            ),
            (
                (269504785, 91252346, 4026532854, 4293918721, 3767009280, 494351),
                hdf5plugin.Zfp(minbits=1, maxbits=16657, maxprec=64, minexp=-1047),
            ),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Zfp._from_filter_options(filter_options)
                self.assertEqual(
                    compression_filter.filter_options, expected_filter.filter_options
                )

    def testZstd(self):
        for filter_options, expected_options in (
            # (clevel,)
            ((), (3,)),
            ((10,), (10,)),
        ):
            with self.subTest(filter_options=filter_options):
                compression_filter = hdf5plugin.Zstd._from_filter_options(
                    filter_options
                )
                self.assertEqual(compression_filter.filter_options, expected_options)


class TestFromFilterOptions(unittest.TestCase):
    """Test from_filter_options function"""

    def test_filter_name(self):
        compression_filter = hdf5plugin.from_filter_options("bzip2", (5,))
        self.assertEqual(compression_filter, hdf5plugin.BZip2(blocksize=5))


class TestFromFilterOptionsRoundtrip(unittest.TestCase):
    """Test from_filter_options function roundtrip"""

    def _test(
        self,
        compression_filter: h5py.filters.FilterRefBase,
        data: numpy.ndarray[Any, Any],
    ):
        with h5py.File("in_memory", "w", driver="core", backing_store=False) as h5f:
            h5f.create_dataset(
                "data",
                data=data,
                chunks=data.shape,
                compression=compression_filter,
            )
            h5f.flush()

            plist = h5f["data"].id.get_create_plist()
            filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]

        self.assertEqual(len(filters), 1)
        filter_id, _, filter_options, _ = filters[0]

        retrieved_filter = hdf5plugin.from_filter_options(filter_id, filter_options)

        self.assertEqual(
            compression_filter,
            retrieved_filter,
            msg=f"{(compression_filter.filter_id, compression_filter.filter_options)} != {(retrieved_filter.filter_id, retrieved_filter.filter_options)}",
        )

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testBitshuffle(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.Bitshuffle(), data)

    @unittest.skipUnless(should_test("blosc"), "Blosc filter not available")
    def testBlosc(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.Blosc(), data)

    @unittest.skipUnless(should_test("blosc2"), "Blosc2 filter not available")
    def testBlosc2(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.Blosc2(), data)

    @unittest.skipUnless(should_test("bzip2"), "BZip2 filter not available")
    def testBZip2(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.BZip2(), data)

    @unittest.skipUnless(should_test("fcidecomp"), "FCIDECOMP filter not available")
    def testFciDecomp(self):
        data = numpy.arange(256**2, dtype=numpy.uint16).reshape(256, 256)
        self._test(hdf5plugin.FciDecomp(), data)

    @unittest.skipUnless(should_test("lz4"), "LZ4 filter not available")
    def testLZ4(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.LZ4(), data)

    @unittest.skipUnless(should_test("sperr"), "Sperr filter not available")
    def testSperr(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.Sperr(), data)

    @unittest.skipUnless(should_test("sz"), "SZ filter not available")
    def testSZ(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.SZ(), data)

    @unittest.skipUnless(should_test("sz3"), "SZ3 filter not available")
    def testSZ3(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.SZ3(), data)

    @unittest.skipUnless(should_test("zfp"), "Zfp filter not available")
    def testZfp(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        # Roundtrip does not work for all parameters including the default
        for mode_name, compression_filter in {
            # rate does not roundtrip
            "precision": hdf5plugin.Zfp(precision=10),
            "accuracy": hdf5plugin.Zfp(accuracy=2**-3),  # roundtrip only for 2^n
            "reversible": hdf5plugin.Zfp(reversible=True),
            "expert": hdf5plugin.Zfp(minbits=2, maxbits=100, maxprec=32, minexp=-10),
        }.items():
            with self.subTest(mode_name):
                self._test(compression_filter, data)

    @unittest.skipUnless(should_test("zstd"), "Zstd filter not available")
    def testZstd(self):
        data = numpy.arange(256**2, dtype=numpy.float32).reshape(256, 256)
        self._test(hdf5plugin.Zstd(), data)


class TestFilterGetConfig(unittest.TestCase):
    """Test filter's get_config method"""

    def testGetConfigRoundtrip(self):
        """Test that filter's get_config method returned value roundtrips"""
        for filter_class in _filters.FILTER_CLASSES:
            with self.subTest(filter=filter_class.filter_name):
                filter_instance = filter_class()
                config = filter_instance.get_config()
                self.assertIsInstance(config, dict)
                cls = cast(type[h5py.filters.FilterRefBase], filter_class)
                self.assertEqual(filter_instance, cls(**config))


class TestFilterProperties(unittest.TestCase):
    """Test filter's parameter properties"""

    def testBitshuffle(self):
        """Test Bitshuffle filter properties"""
        lz4_filter = hdf5plugin.Bitshuffle(nelems=512, cname="lz4")
        self.assertEqual(lz4_filter.nelems, 512)
        self.assertEqual(lz4_filter.cname, "lz4")
        self.assertIsNone(lz4_filter.clevel)

        zstd_filter = hdf5plugin.Bitshuffle(nelems=512, cname="zstd", clevel=5)
        self.assertEqual(zstd_filter.nelems, 512)
        self.assertEqual(zstd_filter.cname, "zstd")
        self.assertEqual(zstd_filter.clevel, 5)

    def testBlosc(self):
        """Test Blosc filter properties"""
        filter_ = hdf5plugin.Blosc(
            cname="zlib", clevel=7, shuffle=hdf5plugin.Blosc.BITSHUFFLE
        )
        self.assertEqual(filter_.cname, "zlib")
        self.assertEqual(filter_.clevel, 7)
        self.assertEqual(filter_.shuffle, hdf5plugin.Blosc.BITSHUFFLE)

    def testBlosc2(self):
        """Test Blosc2 filter properties"""
        filter_ = hdf5plugin.Blosc2(
            cname="zstd", clevel=9, filters=hdf5plugin.Blosc2.NOFILTER
        )
        self.assertEqual(filter_.cname, "zstd")
        self.assertEqual(filter_.clevel, 9)
        self.assertEqual(filter_.filters, hdf5plugin.Blosc2.NOFILTER)

    def testBZip2(self):
        """Test BZip2 filter properties"""
        filter_ = hdf5plugin.BZip2(blocksize=7)
        self.assertEqual(filter_.blocksize, 7)

    def testLZ4(self):
        """Test LZ4 filter properties"""
        filter_ = hdf5plugin.LZ4(nbytes=2048)
        self.assertEqual(filter_.nbytes, 2048)

    def testZstd(self):
        """Test Zstd filter properties"""
        filter_ = hdf5plugin.Zstd(clevel=15)
        self.assertEqual(filter_.clevel, 15)


class TestPackage(unittest.TestCase):
    """Test general features of the hdf5plugin package"""

    def testConstants(self):
        self.assertIsInstance(
            hdf5plugin.FILTERS,
            dict,
        )
        self.assertTrue(
            hdf5plugin.PLUGIN_PATH.startswith(
                os.path.abspath(os.path.dirname(__file__))
            )
        )
        self.assertEqual(
            hdf5plugin.PLUGIN_PATH,
            hdf5plugin.PLUGINS_PATH,
        )

    def testGetConfig(self):
        """Test hdf5plugin.get_config availability"""
        config = hdf5plugin.get_config()
        self.assertIsInstance(config.build_config.openmp, bool)
        self.assertIsInstance(config.build_config.native, bool)
        self.assertIsInstance(config.build_config.sse2, bool)
        self.assertIsInstance(config.build_config.avx2, bool)
        self.assertIsInstance(config.build_config.cpp11, bool)
        self.assertIsInstance(config.build_config.cpp14, bool)
        self.assertIsInstance(config.build_config.embedded_filters, tuple)
        self.assertIsInstance(config.registered_filters, dict)

    def testVersion(self):
        """Test version information"""
        self.assertIsInstance(hdf5plugin.version, str)
        version_info = hdf5plugin.version_info
        self.assertIsInstance(version_info.major, int)
        self.assertIsInstance(version_info.minor, int)
        self.assertIsInstance(version_info.micro, int)
        self.assertIsInstance(version_info.releaselevel, str)
        self.assertIsInstance(version_info.serial, int)


class TestRegisterFilter(BaseTestHDF5PluginRW):
    """Test usage of the register function"""

    def _simple_test(self, filter_name: str):
        if filter_name == "fcidecomp":
            self._test("fcidecomp", dtype=numpy.uint8)
        elif filter_name in ("sz", "zfp"):
            self._test(filter_name, dtype=numpy.float32, lossless=False)
        else:
            self._test(filter_name)

    @unittest.skipUnless(BUILD_CONFIG.embedded_filters, "No embedded filters")
    def test_register_single_filter_by_name(self):
        """Re-register embedded filters one at a time given their name"""
        for filter_name in BUILD_CONFIG.embedded_filters:
            with self.subTest(name=filter_name):
                status = hdf5plugin.register(filter_name, force=True)
                self.assertTrue(status)
                self._simple_test(filter_name)

    @unittest.skipUnless(BUILD_CONFIG.embedded_filters, "No embedded filters")
    def test_register_single_filter_by_id(self):
        """Re-register embedded filters one at a time given their ID"""
        for filter_name in BUILD_CONFIG.embedded_filters:
            with self.subTest(name=filter_name):
                filter_class = hdf5plugin.get_filters(filter_name)[0]
                assert filter_class.filter_id is not None
                status = hdf5plugin.register(filter_class.filter_id, force=True)
                self.assertTrue(status)
                self._simple_test(filter_name)

    @unittest.skipUnless(BUILD_CONFIG.embedded_filters, "No embedded filters")
    def test_register_all_filters(self):
        """Re-register embedded filters all at once"""
        hdf5plugin.register()
        for filter_name in BUILD_CONFIG.embedded_filters:
            with self.subTest(name=filter_name):
                self._simple_test(filter_name)


class TestGetFilters(unittest.TestCase):
    """Test get_filters function"""

    def testDefault(self):
        """Get all filters: get_filters()"""
        filters = hdf5plugin.get_filters()
        self.assertEqual(filters, _filters.FILTER_CLASSES)

    def testRegistered(self):
        """Get registered filters: get_filters("registered")"""
        filters = hdf5plugin.get_filters("registered")
        self.assertTrue(set(filters).issubset(_filters.FILTER_CLASSES))

        filter_names = {f.filter_name for f in filters}
        registered_names = set(hdf5plugin.get_config().registered_filters.keys())
        self.assertEqual(filter_names, registered_names)

    def testSelection(self):
        """Get selected filters"""
        tests: dict[
            int | str | tuple[int | str, ...],
            tuple[type[h5py.filters.FilterRefBase], ...],
        ] = {
            "blosc": (hdf5plugin.Blosc,),
            ("blosc", "zfp"): (hdf5plugin.Blosc, hdf5plugin.Zfp),
            307: (hdf5plugin.BZip2,),
            ("blosc", 307): (hdf5plugin.Blosc, hdf5plugin.BZip2),
        }
        for filters, ref in tests.items():
            with self.subTest(filters=filters):
                self.assertEqual(hdf5plugin.get_filters(filters), ref)


class TestSZ(unittest.TestCase):
    """Specific tests for SZ compression"""

    @unittest.skipUnless(should_test("sz"), "SZ filter not available")
    def testAbsoluteMode(self):
        """Test SZ's absolute mode is within required tolerance

        See https://github.com/silx-kit/hdf5plugin/issues/267
        """
        tolerance = 0.01

        numpy.random.seed(0)
        data = numpy.random.random(size=(1000, 25, 25)).astype(numpy.float32)

        compression = hdf5plugin.SZ(absolute=tolerance)

        with tempfile.TemporaryDirectory() as tempdir:
            filename = os.path.join(tempdir, "testsz.h5")
            with h5py.File(filename, "w", driver="core", backing_store=False) as f:
                f.create_dataset(
                    "var", data=data, chunks=data.shape, compression=compression
                )
                f.flush()

                recovered_data = f["var"][:]

        self.assertTrue(
            numpy.allclose(data, recovered_data, atol=tolerance),
            f"Condition not fulfilled for {tolerance} -> {numpy.max(numpy.abs(recovered_data - data))}",
        )


class TestBlosc2Plugins(unittest.TestCase):
    """Specific tests for Blosc2 compression with Blosc2 plugins"""

    def setUp(self):
        if not should_test("blosc2"):
            self.skipTest("Blosc2 filter not available")
        if blosc2 is None:
            self.skipTest("Blosc2 package not available")

    def _readback_hdf5_blosc2_dataset(
        self,
        data: numpy.ndarray[Any, Any],
        blocks: tuple[int, ...] = None,
        **cparams,
    ):
        """Compress data with blosc2, write it as HDF5 file with direct chunk write and read it back with h5py

        :param data: data array to compress
        :param blocks: Blosc2 block shape
        :param cparams: Blosc2 compression parameters
        """
        # Convert data to a blosc2 array: This is where compression happens
        blosc_array = blosc2.asarray(
            data,
            chunks=data.shape,
            blocks=blocks,
            cparams=cparams,
        )

        # Write blosc2 array as a hdf5 dataset
        with io.BytesIO() as buffer, h5py.File(buffer, "w") as f:
            dataset = f.create_dataset(
                "data",
                shape=data.shape,
                dtype=data.dtype,
                chunks=data.shape,
                compression=hdf5plugin.Blosc2(),
            )
            dataset.id.write_direct_chunk(
                (0,) * data.ndim,
                blosc_array.schunk.to_cframe(),
            )
            f.flush()

            return dataset[()]

    def test_blosc2_filter_int_trunc(self):
        """Read blosc2 dataset written with int truncate filter plugin"""
        data = numpy.arange(2**16, dtype=numpy.int16)

        removed_bits = 2
        read_data = self._readback_hdf5_blosc2_dataset(
            data,
            codec=blosc2.Codec.ZSTD,
            filters=[blosc2.Filter.INT_TRUNC],
            filters_meta=[-removed_bits],
        )
        assert numpy.allclose(read_data, data, rtol=0.0, atol=2**removed_bits)

    def test_blosc2_codec_zfp(self):
        """Read blosc2 dataset written with zfp codec plugin"""
        data = numpy.outer(numpy.arange(128), numpy.arange(128)).astype(numpy.float32)

        read_data = self._readback_hdf5_blosc2_dataset(
            data,
            codec=blosc2.Codec.ZFP_PREC,
            codec_meta=8,
            filters=[],
            filters_meta=[],
            splitmode=blosc2.SplitMode.NEVER_SPLIT,
        )
        assert numpy.allclose(read_data, data, rtol=1e-3, atol=0)

    @unittest.skipIf(
        importlib.util.find_spec("blosc2_grok") is None,
        "blosc2_grok package is not available",
    )
    def test_blosc2_codec_grok(self):
        """Read blosc2 dataset written with blosc2-grok external codec plugin"""
        shape = 10, 128, 128
        data = numpy.arange(numpy.prod(shape), dtype=numpy.uint16).reshape(shape)

        read_data = self._readback_hdf5_blosc2_dataset(
            data,
            blocks=(1,) + data.shape[1:],  # 1 block per slice
            codec=blosc2.Codec.GROK,
            # Disable the filters and the splitmode, because these don't work with grok.
            filters=[],
            splitmode=blosc2.SplitMode.NEVER_SPLIT,
        )
        assert numpy.array_equal(read_data, data)


def suite() -> unittest.TestSuite:
    test_suite = unittest.TestSuite()
    for cls in (
        TestHDF5PluginRW,
        TestStrings,
        TestFromFilterOptionsMethods,
        TestFromFilterOptions,
        TestFromFilterOptionsRoundtrip,
        TestFilterGetConfig,
        TestFilterProperties,
        TestPackage,
        TestRegisterFilter,
        TestGetFilters,
        TestSZ,
        TestBlosc2Plugins,
    ):
        test_suite.addTest(unittest.TestLoader().loadTestsFromTestCase(cls))
    return test_suite


def run_tests(*args, **kwargs) -> bool:
    """Run test complete test_suite"""
    runner = unittest.TextTestRunner(*args, **kwargs)
    success = runner.run(suite()).wasSuccessful()
    print("Test suite " + ("succeeded" if success else "failed"))
    return success


if __name__ == "__main__":
    import argparse
    import sys

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--verbose", "-v", action="count", default=1, help="Increase verbosity"
    )
    options = parser.parse_args()
    sys.exit(0 if run_tests(verbosity=options.verbose) else 1)
