# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2019-2022 European Synchrotron Radiation Facility
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
import os
import shutil
import tempfile
import unittest
import numpy
import h5py
import hdf5plugin


from hdf5plugin import _filters


BUILD_CONFIG = hdf5plugin.get_config().build_config


def should_test(filter_name):
    """Returns True if the given filter should be tested"""
    filter_id = hdf5plugin.FILTERS[filter_name]
    return filter_name in BUILD_CONFIG.embedded_filters or h5py.h5z.filter_avail(filter_id)


class BaseTestHDF5PluginRW(unittest.TestCase):
    """Base class for testing write/read HDF5 dataset with the plugins"""

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
        data = numpy.ones((1000,), dtype=dtype).reshape(100, 10)
        filename = os.path.join(self.tempdir, "test_" + filter_name + ".h5")

        args = {"blosc": hdf5plugin.Blosc,
                "bshuf": hdf5plugin.Bitshuffle,
                "bzip2": hdf5plugin.BZip2,
                "lz4": hdf5plugin.LZ4,
                "fcidecomp": hdf5plugin.FciDecomp,
                "sz": hdf5plugin.SZ,
                "sz3": hdf5plugin.SZ3,
                "zfp": hdf5plugin.Zfp,
                "zstd": hdf5plugin.Zstd,
                }[filter_name](**options)

        # Write
        f = h5py.File(filename, "w")
        f.create_dataset("data", data=data, chunks=data.shape, **args)
        f.close()

        # Read
        with h5py.File(filename, "r") as f:
            saved = f['data'][()]
            plist = f['data'].id.get_create_plist()
            filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]

            if h5py.version.version_tuple >= (2, 10):  # Need read_direct_chunk
                # Read chunk raw (compressed) data
                chunk = f['data'].id.read_direct_chunk((0,) * data.ndim)[1]

                if compressed:  # Check if chunk is actually compressed
                    self.assertLess(len(chunk), data.nbytes)
                else:
                    self.assertEqual(len(chunk), data.nbytes)

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

    @unittest.skipUnless(should_test("bshuf"), "Bitshuffle filter not available")
    def testBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test('bshuf')  # Default options

        compression_ids = {
            'none': 0,
            'lz4': 2,
            'zstd': 3
        }

        # Specify options
        for cname in ('none', 'lz4', 'zstd'):
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(cname=cname, dtype=dtype, nelems=nelems):
                        filter_ = self._test('bshuf', dtype, compressed=cname != 'none', nelems=nelems, cname=cname)
                        self.assertEqual(filter_[2][3:5], (nelems, compression_ids[cname]))

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
                 #{'lossless': False, 'relative': 0.0001},
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
        self.assertIsInstance(config.build_config.embedded_filters, tuple)
        self.assertIsInstance(config.registered_filters, dict)

    def testDeprecatedConfig(self):
        """Test hdf5plugin.config availability"""
        config = hdf5plugin.config
        self.assertIsInstance(config.openmp, bool)
        self.assertIsInstance(config.native, bool)
        self.assertIsInstance(config.sse2, bool)
        self.assertIsInstance(config.avx2, bool)
        self.assertIsInstance(config.cpp11, bool)
        self.assertIsInstance(config.embedded_filters, tuple)

    def testVersion(self):
        """Test version information"""
        self.assertIsInstance(hdf5plugin.version, str)
        self.assertIsInstance(hdf5plugin.strictversion, str)
        self.assertIsInstance(hdf5plugin.hexversion, int)
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

    @unittest.skipIf(h5py.version.version_tuple < (2, 10), "h5py<2.10: unregister_filer not available")
    @unittest.skipUnless(BUILD_CONFIG.embedded_filters, "No embedded filters")
    def test_register_single_filter_by_name(self):
        """Re-register embedded filters one at a time given their name"""
        for filter_name in BUILD_CONFIG.embedded_filters:
            with self.subTest(name=filter_name):
                status = hdf5plugin.register(filter_name, force=True)
                self.assertTrue(status)
                self._simple_test(filter_name)

    @unittest.skipIf(h5py.version.version_tuple < (2, 10), "h5py<2.10: unregister_filer not available")
    @unittest.skipUnless(BUILD_CONFIG.embedded_filters, "No embedded filters")
    def test_register_single_filter_by_id(self):
        """Re-register embedded filters one at a time given their ID"""
        for filter_name in BUILD_CONFIG.embedded_filters:
            with self.subTest(name=filter_name):
                filter_class = hdf5plugin.get_filters(filter_name)[0]
                status = hdf5plugin.register(filter_class.filter_id, force=True)
                self.assertTrue(status)
                self._simple_test(filter_name)

    @unittest.skipIf(h5py.version.version_tuple < (2, 10), "h5py<2.10: unregister_filer not available")
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


def suite():
    test_suite = unittest.TestSuite()
    for cls in (TestHDF5PluginRW, TestPackage, TestRegisterFilter, TestGetFilters):
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
