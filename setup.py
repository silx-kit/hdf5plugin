# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2004-2016 European Synchrotron Radiation Facility
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
__authors__ = ["V.A. Sole"]
__license__ = "MIT"
__date__ = "06/07/2016"

from setuptools import setup

version="1.0.1"
name = "hdf5plugin"
author = "ESRF - Data Analysis Unit"
description = "HDF5 Plugins for windows"
f = open("README.rst")
long_description=f.read()
f.close()
classifiers = ["Development Status :: 3 - Alpha",
               "Environment :: Console",
               "Environment :: Win32 (MS Windows)",
               "Intended Audience :: Education",
               "Intended Audience :: Science/Research",
               "License :: OSI Approved :: MIT License",
               "Natural Language :: English",
               "Operating System :: Microsoft :: Windows",
               "Programming Language :: Python :: 2.7",
               "Programming Language :: Python :: 3.5",
               "Topic :: Scientific/Engineering :: Physics",
               "Topic :: Software Development :: Libraries :: Python Modules",
               ]
package_data = {'hdf5plugin': ["VS2008/x86/*" ,"VS2008/x64/*",
                               "VS2015/x86/*", "VS2015/x64/*"]}

if __name__ == "__main__":
    setup(name=name,
          version=version,
          author=author,
          classifiers=classifiers,
          description=description,
          long_description=long_description,
          package_data=package_data,
          packages=[name],
          )
          
