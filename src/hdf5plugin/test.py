# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2019-2020 European Synchrotron Radiation Facility
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


class TestHDF5PluginRW(unittest.TestCase):
    """Test write/read a HDF5 file with the plugins"""

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
                "lz4": hdf5plugin.LZ4,
                "fcidecomp": hdf5plugin.FciDecomp,
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

    def testBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test('bshuf')  # Default options

        # Specify options
        for lz4 in (False, True):
            for dtype in (numpy.int8, numpy.int16, numpy.int32, numpy.int64):
                for nelems in (1024, 2048):
                    with self.subTest(lz4=lz4, dtype=dtype, nelems=nelems):
                        filter_ = self._test('bshuf', dtype, compressed=lz4, nelems=nelems, lz4=lz4)
                        self.assertEqual(filter_[2][3:], (nelems, 2 if lz4 else 0))

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
                        if cname == 'snappy' and not hdf5plugin.config.cpp11:
                            self.skipTest("snappy unavailable without C++11")
                        filter_ = self._test(
                            'blosc',
                            compressed=clevel!=0,  # No compression for clevel=0
                            cname=cname,
                            clevel=clevel,
                            shuffle=shuffle)
                        self.assertEqual(
                            filter_[2][4:], (clevel, shuffle, compression_id))

    def testLZ4(self):
        """Write/read test with lz4 filter plugin"""
        self._test('lz4')

        # Specify options
        filter_ = self._test('lz4', nbytes=1024)
        self.assertEqual(filter_[2], (1024,))

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.FCIDECOMP_ID),
                         "FCIDECOMP filter not available")
    def testFciDecomp(self):
        """Write/read test with fcidecomp filter plugin"""
        # Test with supported datatypes
        for dtype in (numpy.uint8, numpy.uint16, numpy.int8, numpy.int16):
            with self.subTest(dtype=dtype):
                self._test('fcidecomp', dtype=dtype)

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.ZFP_ID),
                         "ZFP filter not available")
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

    def testZstd(self):
        """Write/read test with Zstd filter plugin"""
        self._test('zstd')


class TestPackage(unittest.TestCase):
    """Test general features of the hdf5plugin package"""

    def testConfig(self):
        """Test hdf5plugin.config availability"""
        config = hdf5plugin.config
        self.assertIsInstance(config.openmp, bool)
        self.assertIsInstance(config.native, bool)
        self.assertIsInstance(config.sse2, bool)
        self.assertIsInstance(config.avx2, bool)
        self.assertIsInstance(config.cpp11, bool)

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


def suite():
    test_suite = unittest.TestSuite()
    for cls in (TestHDF5PluginRW, TestPackage):
        test_suite.addTest(unittest.TestLoader().loadTestsFromTestCase(cls))
    return test_suite


def run_tests(*args, **kwargs):
    """Run test complete test_suite"""
    runner = unittest.TextTestRunner(*args, **kwargs)
    success = runner.run(suite()).wasSuccessful()
    print("Test suite " + ("succeeded" if success else "failed"))
    return success


if __name__ == '__main__':
    import sys
    sys.exit(0 if run_tests() else 1)
