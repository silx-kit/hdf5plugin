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
__date__ = "03/10/2019"


from glob import glob
import os
import sys
from setuptools import setup, Extension
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.build_ext import build_ext
from distutils.command.build import build


# Patch bdist_wheel
try:
    from wheel.bdist_wheel import bdist_wheel
except ImportError:
    BDistWheel = None
else:
    from wheel.pep425tags import get_platform

    class BDistWheel(bdist_wheel):
        """Override bdist_wheel to handle as pure python package"""

        def finalize_options(self):
            bdist_wheel.finalize_options(self)
            self.root_is_pure = True
            self.plat_name_supplied = True
            self.plat_name = get_platform()
            self.universal = not sys.platform.startswith('win')
            if not self.universal:
                self.python_tag = 'py' + str(sys.version_info[0])


# Plugins

class Build(build):
    """Build command with extra options used by PluginBuildExt"""

    user_options = [('hdf5=', None, "Custom path to HDF5 (as in h5py)")]
    user_options.extend(build.user_options)

    def initialize_options(self):
        build.initialize_options(self)
        self.hdf5 = None


class PluginBuildExt(build_ext):
    """Build extension command for DLLs that are not Python modules

    This is actually only useful for Windows
    """

    def get_export_symbols(self, ext):
        """Overridden to remove PyInit_* export"""
        return ext.export_symbols

    def get_ext_filename(self, ext_name):
        """Overridden to use .dll as file extension"""
        if sys.platform.startswith('win'):
            return os.path.join(*ext_name.split('.')) + '.dll'
        elif sys.platform.startswith('linux'):
            return os.path.join(*ext_name.split('.')) + '.so'
        elif sys.platform.startswith('darwin'):
            return os.path.join(*ext_name.split('.')) + '.dylib'
        else:
            return build_ext.get_ext_filename(self, ext_name)

    def build_extensions(self):
        """Overridden to tune extensions.

        - select compile args for MSVC and others
        - Set hdf5 directory
        """
        hdf5_dir = self.distribution.get_command_obj("build").hdf5

        prefix = '/' if self.compiler.compiler_type == 'msvc' else '-'

        for e in self.extensions:
            if isinstance(e, HDF5PluginExtension):
                e.set_hdf5_dir(hdf5_dir)

            e.extra_compile_args = [
                arg for arg in e.extra_compile_args if arg.startswith(prefix)]

        build_ext.build_extensions(self)


class HDF5PluginExtension(Extension):
    """Extension adding specific things to build a HDF5 plugin"""

    def __init__(self, name, **kwargs):
        Extension.__init__(self, name, **kwargs)
        if sys.platform.startswith('win'):
            self.sources.append(os.path.join('src', 'register_win32.c'))
            self.export_symbols.append('register_filter')
            self.define_macros.append(('H5_BUILT_AS_DYNAMIC_LIB', None))
            self.libraries.append('hdf5')

        else:
            self.sources.append(os.path.join('src', 'hdf5_dl.c'))
            self.export_symbols.append('init_filter')

        self.define_macros.append(('H5_USE_18_API', None))

    def set_hdf5_dir(self, hdf5_dir=None):
        """Set the HDF5 installation directory to use to build the plugins.

        It should contains an "include" subfolder containing the HDF5 headers,
        and on Windows a "lib" subfolder containing the hdf5.lib file.

        :param Union[str,None] hdf5_dir:
        """
        if hdf5_dir is None:
            hdf5_dir = os.path.join('src', 'hdf5')
            # Add folder containing H5pubconf.h
            if sys.platform.startswith('win'):
                folder = 'windows' if sys.version_info[0] >= 3 else 'windows-2.7'
            else:
                folder = 'darwin' if sys.platform.startswith('darwin') else 'linux'
            self.include_dirs.insert(0, os.path.join(hdf5_dir, 'include', folder))

        if sys.platform.startswith('win'):
            self.library_dirs.insert(0, os.path.join(hdf5_dir, 'lib'))
        self.include_dirs.insert(0, os.path.join(hdf5_dir, 'include'))


def prefix(directory, files):
    """Add a directory as prefix to a list of files.

    :param str directory: Directory to add as prefix
    :param List[str] files: List of relative file path
    :rtype: List[str]
    """
    return ['/'.join((directory, f)) for f in files]


# bitshuffle (+lz4) plugin
# Plugins from https://github.com/kiyo-masui/bitshuffle
# TODO compile flags + openmp
bithsuffle_dir = 'src/bitshuffle'

# Set compile args for both MSVC and others, list is stripped at build time
extra_compile_args = ['-O3', '-ffast-math', '-march=native', '-std=c99']
extra_compile_args += ['/Ox', '/fp:fast']

bithsuffle_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5bshuf",
    sources=prefix(bithsuffle_dir,
        ["src/bshuf_h5plugin.c", "src/bshuf_h5filter.c",
         "src/bitshuffle.c", "src/bitshuffle_core.c",
         "src/iochain.c", "lz4/lz4.c"]),
    depends=prefix(bithsuffle_dir,
        ["src/bitshuffle.h", "src/bitshuffle_core.h",
         "src/iochain.h", 'src/bshuf_h5filter.h',
         "lz4/lz4.h"]),
    include_dirs=prefix(bithsuffle_dir, ['src/', 'lz4/']),
    extra_compile_args=extra_compile_args,
    )


# blosc plugin
# Plugin from https://github.com/Blosc/hdf5-blosc
# c-blosc from https://github.com/Blosc/c-blosc
# TODO compile flags avx2/sse2, snappy
hdf5_blosc_dir = 'src/hdf5-blosc/src/'
blosc_dir = 'src/c-blosc/'

# blosc sources
sources = [f for f in glob(blosc_dir + 'blosc/*.c')
           if 'avx2' not in f and 'sse2' not in f]
depends = [f for f in glob(blosc_dir + 'blosc/*.h')
        if 'avx2' not in f and 'sse2' not in f]
include_dirs = [blosc_dir, blosc_dir + 'blosc']
define_macros = []

# compression libs
# lz4
lz4_sources = glob(blosc_dir + 'internal-complibs/lz4*/*.c')
lz4_depends = glob(blosc_dir + 'internal-complibs/lz4*/*.h')
lz4_include_dirs = glob(blosc_dir + 'internal-complibs/lz4*')

sources += lz4_sources
depends += lz4_depends
include_dirs += lz4_include_dirs
define_macros.append(('HAVE_LZ4', 1))

# snappy
# TODO

#zlib
sources += glob(blosc_dir + 'internal-complibs/zlib*/*.c')
depends += glob(blosc_dir + 'internal-complibs/zlib*/*.h')
include_dirs += glob(blosc_dir + 'internal-complibs/zlib*')
define_macros.append(('HAVE_ZLIB', 1))

# zstd
sources += glob(blosc_dir +'internal-complibs/zstd*/*/*.c')
depends += glob(blosc_dir +'internal-complibs/zstd*/*/*.h')
include_dirs += glob(blosc_dir + 'internal-complibs/zstd*')
include_dirs += glob(blosc_dir + 'internal-complibs/zstd*/common')
define_macros.append(('HAVE_ZSTD', 1))


blosc_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5blosc",
    sources=sources + \
        prefix(hdf5_blosc_dir,['blosc_filter.c', 'blosc_plugin.c']),
    depends=depends + \
        prefix(hdf5_blosc_dir, ['blosc_filter.h', 'blosc_plugin.h']),
    include_dirs=include_dirs + [hdf5_blosc_dir],
    define_macros=define_macros,
    )


# lz4 plugin
# Source from https://github.com/nexusformat/HDF5-External-Filter-Plugins
lz4_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5lz4",
    sources=['src/LZ4/H5Zlz4.c'] + \
            lz4_sources,
    depends=lz4_depends,
    include_dirs=lz4_include_dirs,
    libraries=['Ws2_32'] if sys.platform == 'win32' else [],
    )


extensions=[lz4_plugin,
            bithsuffle_plugin,
            blosc_plugin,
            ]

# setup

# ########## #
# version.py #
# ########## #

def get_version():
    """Returns current version number from version.py file"""
    dirname = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, dirname)
    import version
    sys.path = sys.path[1:]
    return version.strictversion


class build_py(_build_py):
    """
    Enhanced build_py which copies version.py to <PROJECT>._version.py
    """
    def find_package_modules(self, package, package_dir):
        modules = _build_py.find_package_modules(self, package, package_dir)
        if package == PROJECT:
            modules.append((PROJECT, '_version', 'version.py'))
        return modules


PROJECT = 'hdf5plugin'
author = "ESRF - Data Analysis Unit"
description = "HDF5 Plugins for windows,MacOS and linux"
url='https://github.com/silx-kit/hdf5plugin'
f = open("README.rst")
long_description=f.read()
f.close()
license = "https://github.com/silx-kit/hdf5plugin/blob/master/LICENSE"
classifiers = ["Development Status :: 4 - Beta",
               "Environment :: Console",
               "Environment :: MacOS X",
               "Environment :: Win32 (MS Windows)",
               "Intended Audience :: Education",
               "Intended Audience :: Science/Research",
               "License :: OSI Approved :: MIT License",
               "License :: OSI Approved :: BSD License",
               "License :: OSI Approved :: zlib/libpng License",
               "Natural Language :: English",
               "Operating System :: POSIX :: Linux",
               "Operating System :: MacOS",
               "Operating System :: Microsoft :: Windows",
               "Programming Language :: Python :: 2.7",
               "Programming Language :: Python :: 3.4",
               "Programming Language :: Python :: 3.5",
               "Programming Language :: Python :: 3.6",
               "Programming Language :: Python :: 3.7",
               "Topic :: Software Development :: Libraries :: Python Modules",
               ]
cmdclass = dict(build=Build,
                build_ext=PluginBuildExt,
                build_py=build_py)
if BDistWheel is not None:
    cmdclass['bdist_wheel'] = BDistWheel


if __name__ == "__main__":
    setup(name=PROJECT,
          version=get_version(),
          author=author,
          url=url,
          classifiers=classifiers,
          description=description,
          long_description=long_description,
          license=license,
          packages=[PROJECT],
          ext_modules=extensions,
          install_requires=['h5py'],
          setup_requires=['setuptools'],
          cmdclass=cmdclass,
          )

