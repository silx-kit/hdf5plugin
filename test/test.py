# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2019 European Synchrotron Radiation Facility
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
__date__ = "30/09/2019"


import os
import shutil
import sys
import tempfile
import unittest
from distutils.version import LooseVersion

import numpy
import h5py
import hdf5plugin


class TestHDF5PluginRead(unittest.TestCase):
    """Test reading existing files with compressed data"""

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


class TestHDF5PluginRW(unittest.TestCase):
    """Test write/read a HDF5 file with the plugins"""

    @classmethod
    def setUpClass(cls):
        cls.tempdir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tempdir)

    def _test(self, filter_name):
        """Run test for a particular filter

        :param str filter_name: The name of the filter to use
        """
        data = numpy.arange(100, dtype='float32')
        filename = os.path.join(self.tempdir, "test_" + filter_name + ".h5")

        # Write
        f = h5py.File(filename, "w")
        f.create_dataset("data",
                         data=data,
                         compression=hdf5plugin.FILTERS[filter_name])
        f.close()

        # Read
        f = h5py.File(filename, "r")
        saved = f['data'][()]
        plist = f['data'].id.get_create_plist()
        filters = [plist.get_filter(i) for i in range(plist.get_nfilters())]
        f.close()

        self.assertTrue(numpy.array_equal(saved, data))
        self.assertEqual(saved.dtype, data.dtype)

        self.assertEqual(len(filters), 1)
        self.assertEqual(filters[0][0], hdf5plugin.FILTERS[filter_name])

        os.remove(filename)

    def testBitshuffle(self):
        """Write/read test with bitshuffle filter plugin"""
        self._test('bshuf')

    def testBlosc(self):
        """Write/read test with blosc filter plugin"""
        self._test('blosc')

    def testLZ4(self):
        """Write/read test with lz4 filter plugin"""
        self._test('lz4')


def getSuite():
    testSuite = unittest.TestSuite()
    for cls in (TestHDF5PluginRead, TestHDF5PluginRW):
        testSuite.addTest(
            unittest.TestLoader().loadTestsFromTestCase(cls))
    return testSuite


def test():
    return unittest.TextTestRunner(verbosity=2).run(getSuite())


if __name__ == "__main__":
    result = test()
    if result.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)
