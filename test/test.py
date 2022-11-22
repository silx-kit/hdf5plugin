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
__date__ = "22/11/2022"


import os
import unittest

import numpy
import h5py
import hdf5plugin
from hdf5plugin.test import suite as hdf5plugin_suite


class TestHDF5PluginRead(unittest.TestCase):
    """Test reading existing files with compressed data"""

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
        expected_shape = (50, 2167, 2070)
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
        expected_shape = (1, 2167, 2070)
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
        expected_shape = (60, 30)
        expected_data = numpy.arange(1800).astype(numpy.int16).reshape(60, 30)
        self.assertTrue(data.shape[0] == 60, "Incorrect shape")
        self.assertTrue(data.shape[1] == 30, "Incorrect shape")
        self.assertTrue(data.dtype == expected_data.dtype, "Incorrect type")
        self.assertTrue(numpy.alltrue(data == expected_data),
                                      "Incorrect values read")

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
            self.assertFalse(numpy.alltrue(original == compressed),
                                      "Values should not be identical")
            self.assertTrue(numpy.allclose(original, compressed),
                                      "Values should be close")

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
            self.assertTrue(numpy.alltrue(original[0:8] == compressed[0, 0, :8]),
                                      "Values should not be identical")

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
