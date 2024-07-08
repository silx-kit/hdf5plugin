# coding: utf-8
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
"""Provides tests """
from __future__ import annotations

import importlib.util
import io
import os
import shutil
import tempfile
import unittest
import numpy
import h5py
import hdf5plugin

try:
    import blosc2
except ImportError:
    blosc2 = None

from hdf5plugin import _filters


BUILD_CONFIG = hdf5plugin.get_config().build_config


def should_test(filter_name):
    """Returns True if the given filter should be tested"""
    filter_id = hdf5plugin.FILTERS[filter_name]
    return filter_name in BUILD_CONFIG.embedded_filters or h5py.h5z.filter_avail(filter_id)


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

    def _test(self,
              filter_name,
              dtype=numpy.int32,
              lossless=True,
              compressed=True,
              **options):
        """Run test for a particular filter

        :param str filter_name: The name of the filter to use
        :param Union[None,tuple(int)] options:
            create_dataset's compression_opts argument
        :return: The tuple describing the filter
        """
        data = numpy.ones((self._data_natoms,), dtype=dtype).reshape(self._data_shape)
        filename = os.path.join(self.tempdir, "test_" + filter_name + ".h5")

        compression_class = {
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
        }[filter_name]

        # Write
        f = h5py.File(filename, "w")
        f.create_dataset("data", data=data, chunks=data.shape, compression=compression_class(**options))
        f.close()

        # Read
        with h5py.File(filename, "r") as f:
            saved = f['data'][()]
            plist = f['data'].id.get_create_plist()
            filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]

            # Read chunk raw (compressed) data
            chunk = f['data'].id.read_direct_chunk((0,) * data.ndim)[1]

            if compressed is True:  # Check if chunk is actually compressed
                self.assertLess(len(chunk), data.nbytes)
            elif compressed is False:
                self.assertEqual(len(chunk), data.nbytes)
            else:
                assert compressed == 'nocheck'

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
        self._test('bshuf')  # Default options

        # Specify options
        for lz4 in (False, True):
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(lz4=lz4, dtype=dtype, nelems=nelems):
                        filter_ = self._test('bshuf', dtype, compressed=lz4, nelems=nelems, lz4=lz4)
                        self.assertEqual(filter_[2][3:], (nelems, 2 if lz4 else 0))

    def _get_bitshuffle_version(self):
        filename = os.path.join(self.tempdir, "get_bitshuffle_version.h5")
        with h5py.File(filename, "w", driver="core", backing_store=False) as h5f:
            h5f.create_dataset("data", numpy.arange(10), compression=hdf5plugin.Bitshuffle())
            plist = h5f["data"].id.get_create_plist()
            assert plist.get_nfilters() == 1
            filter_ = plist.get_filter(0)
            assert filter_[0] == hdf5plugin.BSHUF_ID
            return tuple(filter_[2][:2])

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test('bshuf')  # Default options

        compressions = {  # Compressor name: Compressor ID
            'none': 0,
            'lz4': 2,
        }
        if self._get_bitshuffle_version() >= (0, 4):
            compressions['zstd'] = 3

        # Specify options
        for cname, compression_id in compressions.items():
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(cname=cname, dtype=dtype, nelems=nelems):
                        filter_ = self._test('bshuf', dtype, compressed=cname != 'none', nelems=nelems, cname=cname)
                        self.assertEqual(filter_[2][3:5], (nelems, compression_id))

    @unittest.skipUnless(should_test("blosc"), "Blosc filter not available")
    def testBlosc(self):
        """Write/read test with blosc filter plugin"""
        self._test('blosc')  # Default options

        # Specify options
        shuffles = (hdf5plugin.Blosc.NOSHUFFLE,
                    hdf5plugin.Blosc.SHUFFLE,
                    hdf5plugin.Blosc.BITSHUFFLE)
        compress = 'blosclz', 'lz4', 'lz4hc', 'snappy', 'zlib', 'zstd'
        for compression_id, cname in enumerate(compress):
            for shuffle in shuffles:
                for clevel in range(10):
                    with self.subTest(compression=cname,
                                      shuffle=shuffle,
                                      clevel=clevel):
                        if cname == 'snappy' and not BUILD_CONFIG.cpp11:
                            self.skipTest("snappy unavailable without C++11")
                        filter_ = self._test(
                            'blosc',
                            compressed=clevel != 0,  # No compression for clevel=0
                            cname=cname,
                            clevel=clevel,
                            shuffle=shuffle)
                        self.assertEqual(
                            filter_[2][4:], (clevel, shuffle, compression_id))

    @unittest.skipUnless(should_test("blosc2"), "Blosc2 filter not available")
    def testBlosc2(self):
        """Write/read test with blosc2 filter plugin"""
        self._test('blosc2')  # Default options

        # Specify options
        tested_filters = (
            hdf5plugin.Blosc2.NOFILTER,
            hdf5plugin.Blosc2.SHUFFLE,
            hdf5plugin.Blosc2.BITSHUFFLE,
        )
        compress = 'blosclz', 'lz4', 'lz4hc', 'unused', 'zlib', 'zstd'
        for compression_id, cname in enumerate(compress):
            if cname == 'unused':
                continue
            for filters in tested_filters:
                for clevel in range(10):
                    with self.subTest(compression=cname,
                                      filters=filters,
                                      clevel=clevel):
                        filter_ = self._test(
                            'blosc2',
                            compressed='nocheck' if clevel == 0 else True,  # For clevel=0, chunks are larger
                            cname=cname,
                            clevel=clevel,
                            filters=filters)
                        filter_params = (clevel, filters, compression_id)
                        if len(self._data_shape) >= 2:
                            # Chunk shape passed to filter code
                            filter_params += (len(self._data_shape),) + self._data_shape
                        self.assertEqual(filter_[2][4:], filter_params)

    @unittest.skipUnless(should_test("bzip2"), "BZip2 filter not available")
    def testBZip2(self):
        """Write/read test with BZip2 filter plugin"""
        self._test('bzip2')  # Default options

        # Specify options
        for blocksize in range(1, 10):
            with self.subTest(blocksize=blocksize):
                filter_ = self._test('bzip2', blocksize=blocksize)
                self.assertEqual(filter_[2][0], blocksize)

    @unittest.skipUnless(should_test("lz4"), "LZ4 filter not available")
    def testLZ4(self):
        """Write/read test with lz4 filter plugin"""
        self._test('lz4')

        # Specify options
        filter_ = self._test('lz4', nbytes=1024)
        self.assertEqual(filter_[2], (1024,))

    @unittest.skipUnless(should_test("fcidecomp"), "FCIDECOMP filter not available")
    def testFciDecomp(self):
        """Write/read test with fcidecomp filter plugin"""
        # Test with supported datatypes
        for dtype in (numpy.uint8, numpy.uint16, numpy.int8, numpy.int16):
            with self.subTest(dtype=dtype):
                self._test('fcidecomp', dtype=dtype)

    @unittest.skipUnless(should_test("sperr"), "Sperr filter not available")
    def testSperr(self):
        """Write/read test with Sperr filter plugin"""
        tests = [
            {'lossless': False, 'rate': 16},
            {'lossless': False, 'rate': 16, 'swap': True},
            {'lossless': False, 'peak_signal_to_noise_ratio': 1e-4},
            {'lossless': False, 'peak_signal_to_noise_ratio': 1e-4, 'swap': True},
            {'lossless': False, 'absolute': 1e-4},
            {'lossless': False, 'absolute': 1e-4, 'swap': True}
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test('sperr', dtype=dtype, **options)

    @unittest.skipUnless(should_test("sz"), "SZ filter not available")
    def testSZ(self):
        """Write/read test with SZ filter plugin"""
        # TODO: Options mission
        tests = [{'lossless': False, 'absolute': 0.0001},
                 {'lossless': False, 'relative': 0.01},
                 ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test('sz', dtype=dtype, **options)

    @unittest.skipUnless(should_test("sz3"), "SZ3 filter not available")
    def testSZ3(self):
        """Write/read test with SZ3 filter plugin"""
        # TODO: Options mission
        tests = [{'lossless': False, 'absolute': 0.001},
                 # {'lossless': False, 'relative': 0.0001},
                 ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test('sz3', dtype=dtype, **options)

    @unittest.skipUnless(should_test("zfp"), "ZFP filter not available")
    def testZfp(self):
        """Write/read test with zfp filter plugin"""
        tests = [
            {'lossless': False},  # Default config
            {'lossless': False, 'rate': 10.0},  # Fixed-rate
            {'lossless': False, 'precision': 10},  # Fixed-precision
            {'lossless': False, 'accuracy': 1e-8},  # Fixed-accuracy
            {'lossless': True, 'reversible': True},  # Reversible
            # Expert: with default parameters
            {'lossless': False, 'minbits': 1, 'maxbits': 16657, 'maxprec': 64, 'minexp': -1074},
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test('zfp', dtype=dtype, **options)

        self._test('zfp', dtype=numpy.int32, reversible=True)

    @unittest.skipUnless(should_test("zstd"), "Zstd filter not available")
    def testZstd(self):
        """Write/read test with Zstd filter plugin"""
        self._test('zstd')
        tests = [
            {'clevel': 3},
            {'clevel': 22}
        ]
        for options in tests:
            for dtype in (numpy.float32, numpy.float64):
                with self.subTest(options=options, dtype=dtype):
                    self._test('zstd', dtype=dtype, **options)


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

    def _simple_test(self, filter_name):
        if filter_name == 'fcidecomp':
            self._test('fcidecomp', dtype=numpy.uint8)
        elif filter_name in ('sz', 'zfp'):
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

        filter_names = set(f.filter_name for f in filters)
        registered_names = set(hdf5plugin.get_config().registered_filters.keys())
        self.assertEqual(filter_names, registered_names)

    def testSelection(self):
        """Get selected filters"""
        tests = {
            'blosc': (hdf5plugin.Blosc,),
            ('blosc', 'zfp'): (hdf5plugin.Blosc, hdf5plugin.Zfp),
            307: (hdf5plugin.BZip2,),
            ('blosc', 307): (hdf5plugin.Blosc, hdf5plugin.BZip2),
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
            with h5py.File(filename, 'w', driver="core", backing_store=False) as f:
                f.create_dataset('var', data=data, chunks=data.shape, **compression)
                f.flush()

                recovered_data = f["var"][:]

        self.assertTrue(
            numpy.allclose(data, recovered_data, atol=tolerance),
            f"Condition not fulfilled for {tolerance} -> {numpy.max(numpy.abs(recovered_data - data))}"
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
            data: numpy.ndarray,
            blocks: tuple[int, ...] | None = None,
            **cparams
    ) -> numpy.ndarray:
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
        with io.BytesIO() as buffer:
            with h5py.File(buffer, 'w') as f:
                dataset = f.create_dataset(
                    'data',
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

    @unittest.skipIf(importlib.util.find_spec("blosc2_grok") is None, "blosc2_grok package is not available")
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


def suite():
    test_suite = unittest.TestSuite()
    for cls in (TestHDF5PluginRW, TestPackage, TestRegisterFilter, TestGetFilters, TestSZ, TestBlosc2Plugins):
        test_suite.addTest(unittest.TestLoader().loadTestsFromTestCase(cls))
    return test_suite


def run_tests(*args, **kwargs):
    """Run test complete test_suite"""
    runner = unittest.TextTestRunner(*args, **kwargs)
    success = runner.run(suite()).wasSuccessful()
    print("Test suite " + ("succeeded" if success else "failed"))
    return success


if __name__ == '__main__':
    import argparse
    import sys
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", "-v", action="count", default=1, help="Increase verbosity")
    options = parser.parse_args()
    sys.exit(0 if run_tests(verbosity=options.verbose) else 1)
