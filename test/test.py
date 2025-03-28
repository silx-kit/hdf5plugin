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
__authors__ = ["V.A. Sole", "T. Vincent"]
__license__ = "MIT"
__date__ = "06/12/2022"


import os
import unittest
import tempfile
import shutil

import numpy
import h5py
import hdf5plugin
from hdf5plugin.test import suite as hdf5plugin_suite


class TestHDF5PluginRead(unittest.TestCase):
    """Test reading existing files with compressed data"""

    @classmethod
    def setUpClass(cls):
        cls.tempdir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tempdir)

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.BLOSC_ID),
                         "Blosc filter not available")
    def testBlosc(self):
        """Test reading Blosc compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        # the blosc.h5 is in fact the example.h5 file generated
        # using the example.c file from the blosc respository.
        fname = os.path.join(dirname, "blosc.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        h5 = h5py.File(fname, "r")
        data = h5["/dset"][:]
        h5.close()
        expected_shape = (100, 100, 100)
        self.assertTrue(data.shape[0] == 100, "Incorrect shape")
        self.assertTrue(data.shape[1] == 100, "Incorrect shape")
        self.assertTrue(data.shape[2] == 100, "Incorrect shape")

        target = numpy.arange(numpy.prod(expected_shape),
                              dtype=numpy.float64)
        target.shape = expected_shape
        self.assertTrue(numpy.allclose(data, target), "Incorrect readout")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.LZ4_ID),
                         "LZ4 filter not available")
    def testLZ4(self):
        """Test reading lz4 compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "lz4.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        h5 = h5py.File(fname, "r")
        data = h5["/entry/data"][:]
        h5.close()
        self.assertTrue(data.shape[0] == 50, "Incorrect shape")
        self.assertTrue(data.shape[1] == 2167, "Incorrect shape")
        self.assertTrue(data.shape[2] == 2070, "Incorrect shape")
        self.assertTrue(data[21, 1911, 1549] == 3141, "Incorrect value")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.BSHUF_ID),
                         "Bitshuffle filter not available")
    def testBitshuffle(self):
        """Test reading bitshuffle compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "bitshuffle.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        h5 = h5py.File(fname, "r")
        data = h5["/entry/data/data"][:]
        h5.close()
        self.assertTrue(data.shape[0] == 1, "Incorrect shape")
        self.assertTrue(data.shape[1] == 2167, "Incorrect shape")
        self.assertTrue(data.shape[2] == 2070, "Incorrect shape")
        self.assertTrue(data[0, 1372, 613] == 922, "Incorrect value")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.FCIDECOMP_ID),
                         "FCIDECOMP filter not available")
    def testFcidecomp(self):
        """Test reading FCIDECOMP compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "fcidecomp.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        h5 = h5py.File(fname, "r")
        data = h5["effective_radiance"][:]
        h5.close()
        expected_data = numpy.arange(1800).astype(numpy.int16).reshape(60, 30)
        self.assertTrue(data.shape[0] == 60, "Incorrect shape")
        self.assertTrue(data.shape[1] == 30, "Incorrect shape")
        self.assertTrue(data.dtype == expected_data.dtype, "Incorrect type")
        self.assertTrue(numpy.all(data == expected_data), "Incorrect values read")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.ZFP_ID),
                         "ZFP filter not available")
    def testZfp(self):
        """Test reading ZFP compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        for fname in ["zfp_050.h5", "zfp_052.h5", "zfp_054.h5"]:
            fname = os.path.join(dirname, fname)
            self.assertTrue(os.path.exists(fname),
                            "Cannot find %s file" % fname)
            h5 = h5py.File(fname, "r")
            original = h5["original"][()]
            compressed = h5["compressed"][()]
            h5.close()
            self.assertTrue(original.shape == compressed.shape,
                            "Incorrect shape")
            self.assertTrue(original.dtype == compressed.dtype,
                            "Incorrect dtype")
            self.assertFalse(numpy.all(original == compressed),
                             "Values should not be identical")
            self.assertTrue(numpy.allclose(original, compressed),
                            "Values should be close")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.SPERR_ID),
                         "Sperr filter not available")
    def testSperrV0_1(self):
        """Test reading Sperr compressed data with v0.1 of the filter"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "sperr_v0.1.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        with h5py.File(fname, "r") as h5:
            compressed = h5["f64_sperr"][()]
            original = h5["f64_original"][()]
        self.assertTrue(original.shape == compressed.shape,
                        "Incorrect shape")
        self.assertTrue(original.dtype == compressed.dtype,
                        "Incorrect dtype")
        self.assertTrue(numpy.allclose(original, compressed, atol=1e-3),
                        "Values should be close")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.SPERR_ID),
                         "Sperr filter not available")
    def testSperrV0_2MissingValue(self):
        """Test reading Sperr compressed data with NaN values"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "sperr_v0.2_missing_val.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        with h5py.File(fname, "r") as h5:
            compressed = h5["f64_sperr"][()]
        self.assertTrue(compressed.shape == (128, 64), "Incorrect shape")
        self.assertTrue(compressed.dtype == numpy.float64, "Incorrect dtype")
        self.assertTrue(numpy.isnan(compressed[0, 1]))
        self.assertTrue(numpy.isnan(compressed[0, 10]))
        self.assertTrue(numpy.isnan(compressed[1, 36]))

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.SZ_ID),
                         "SZ filter not available")
    def testSZ(self):
        """Test reading SZ compressed data"""
        dirname = os.path.abspath(os.path.dirname(__file__))
        for fname in ["sz_testfloat_8_8_128.h5"]:
            fname = os.path.join(dirname, fname)
            self.assertTrue(os.path.exists(fname),
                            "Cannot find %s file" % fname)
            h5 = h5py.File(fname, "r")
            compressed = h5["testfloat"][()]
            h5.close()
            original_shape = (128, 8, 8)
            original = numpy.array([0.23454477,
                                    0.23452051,
                                    0.23450762,
                                    0.23450902,
                                    0.23451449,
                                    0.23453577,
                                    0.23457345,
                                    0.23459189,
                                    0.23454477,
                                    0.23452051,
                                    0.23450762,
                                    0.23450902,
                                    0.23451449,
                                    0.23453577], dtype=numpy.float32)
            self.assertTrue(original_shape == compressed.shape,
                            "Incorrect shape")
            self.assertTrue(original.dtype == compressed.dtype,
                            "Incorrect dtype")
            self.assertTrue(numpy.all(original[0:8] == compressed[0, 0, :8]),
                            "Values should not be identical")

    @unittest.skipUnless(h5py.h5z.filter_avail(hdf5plugin.SZ3_ID),
                         "SZ3 filter not available")
    def testSZ3(self):
        """Test reading and witing SZ3 compressed data"""
        # the file contains original data and data compressed using hdf5plugin under linux
        # the floating point data have been checked against data compressed using h5repack
        # for absolute mode and value 1E-3
        # h5repack -v -f UD=32024,1,9,0,1062232653,3539053052,0,0,0,0,0,0
        # for relative move and value 1E-4
        # h5repack -v -f UD=32024,1,9,1,0,0,1058682594,3944497965,0,0,0,0
        dirname = os.path.abspath(os.path.dirname(__file__))
        fname = os.path.join(dirname, "sz3.h5")
        self.assertTrue(os.path.exists(fname),
                        "Cannot find %s file" % fname)
        for dname in ["testfloat_8_8_128", "testdouble_8_8_128"]:
            h5 = h5py.File(fname, "r")
            original = h5[dname][()]
            # absolute 1E-3
            value = 1E-3
            compressed_name = dname + "_absolute_sz3"
            compressed = h5[compressed_name][()]
            compressed_back = h5[compressed_name + "_back"][()]
            self.assertTrue(original.shape == compressed.shape, "Incorrect shape")
            self.assertFalse(numpy.all(original == compressed),
                             "Values should not be identical")
            self.assertTrue(numpy.allclose(compressed, compressed_back),
                            "Compressed read back values should be identical to compressed data")

            # this also tests the algorithm and not just the plugin
            self.assertTrue(numpy.allclose(original, compressed, atol=value),
                            "Values should be within tolerance")

            # create a compressed file
            output_file = os.path.join(self.tempdir, compressed_name + ".h5")
            with h5py.File(output_file, "w", driver="core", backing_store=False) as h5o:
                h5o.create_dataset("data", data=original, dtype=original.dtype, chunks=original.shape,
                                   compression=hdf5plugin.SZ3(absolute=value))
                output_data = h5o["/data"][()]
            self.assertFalse(numpy.all(original == output_data),
                             "Values should not be identical")
            self.assertTrue(numpy.allclose(original, output_data, atol=value),
                            "Values should be within tolerance")

            # relative 1E-4
            value = 1E-4
            compressed_name = dname + "_relative_sz3"
            compressed = h5[compressed_name][()]
            compressed_back = h5[compressed_name + "_back"][()]
            self.assertTrue(original.shape == compressed.shape, "Incorrect shape")
            self.assertFalse(numpy.all(original == compressed),
                             "Values should not be identical")

            # under windows the results are not identical to linux
            # therefore we cannot check for equality of decompressed values
            self.assertTrue(numpy.allclose(compressed, compressed_back, rtol=0.1 * value),
                            "Relative read back values should be very close to compressed data")

            # create a compressed file
            output_file = os.path.join(self.tempdir, compressed_name + ".h5")
            with h5py.File(output_file, "w", driver="core", backing_store=False) as h5o:
                h5o.create_dataset("data", data=original, dtype=original.dtype, chunks=original.shape,
                                   compression=hdf5plugin.SZ3(relative=value))
                output_data = h5o["/data"][()]
            self.assertFalse(numpy.all(original == output_data),
                             "Values should not be identical")

            # see what relative and absolute differences are acceptable for this mode
            difference = original - compressed_back
            idx = numpy.argmax(abs(difference))
            difference.shape = -1
            rtol = abs(difference[idx] / original.flatten()[idx])

            # TODO: Check why one needs to have such large tolerance
            rtol = rtol * 5
            self.assertTrue(numpy.allclose(original, output_data, rtol=rtol),
                            "Newly compressed data should match original compression quality")
            self.assertTrue(numpy.allclose(compressed_back, output_data, rtol=1.5 * rtol),
                            "Compressed data should be close")
            # L2 Norm
            value = 0.33
            compressed_name = dname + "_norm2_sz3"
            compressed = h5[compressed_name][()]
            compressed_back = h5[compressed_name + "_back"][()]
            self.assertTrue(original.shape == compressed.shape, "Incorrect shape")
            self.assertFalse(numpy.all(original == compressed),
                             "Values should not be identical")
            # Absolute error from L2 norm param from:
            # https://github.com/szcompressor/SZ3/blob/v3.1.8/include/SZ3/utils/Statistic.hpp#L44
            abs_error = numpy.sqrt(3.0 / compressed.ndim) * value
            self.assertTrue(numpy.allclose(compressed, compressed_back, atol=abs_error),
                            "Compressed read back values should be identical to compressed data")

            # create a compressed file
            output_file = os.path.join(self.tempdir, compressed_name + ".h5")
            with h5py.File(output_file, "w", driver="core", backing_store=False) as h5o:
                h5o.create_dataset("data", data=original, dtype=original.dtype, chunks=original.shape,
                                   compression=hdf5plugin.SZ3(norm2=value))
                output_data = h5o["/data"][()]
            self.assertFalse(numpy.all(original == output_data),
                             "Values should not be identical")
            self.assertTrue(numpy.all(compressed == output_data),
                            "Compressed data should be identical")
            self.assertTrue(numpy.allclose(compressed_back, output_data, atol=abs_error),
                            "Newly L2 norm read back values should be identical to compressed data")
            h5.close()


def suite():
    testSuite = unittest.TestSuite()
    testSuite.addTest(unittest.TestLoader().loadTestsFromTestCase(
        TestHDF5PluginRead))
    testSuite.addTest(hdf5plugin_suite())
    return testSuite


if __name__ == "__main__":
    import sys
    result = unittest.TextTestRunner(verbosity=2).run(suite())
    if result.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)
