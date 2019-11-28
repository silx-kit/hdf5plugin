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
import logging
import os
import sys
import tempfile
from setuptools import setup, Extension
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.build_ext import build_ext
from distutils.command.build import build
from distutils.errors import CompileError

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


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
            self.plat_name = get_platform()
            if not sys.platform.startswith('win'):
                self.python_tag = "py2.py3"
            bdist_wheel.finalize_options(self)

        def get_tag(self):
            """Override the python and abi tag generation"""
            return self.python_tag, "none", bdist_wheel.get_tag(self)[-1]


def get_cpu_sse2_avx2():
    """Returns whether SSE2 and AVX2 are available on the current CPU

    :returns: (is SSE2 available, is AVX2 available)
    :rtype: List(bool)
    """
    try:
        import cpuinfo
    except ImportError as e:
        raise e
    except Exception:  # cpuinfo raises Exception for unsupported architectures
        logger.warn(
            "CPU info detection does not support this architecture: SSE2 and AVX2 disabled")
        return False, False
    else:
        cpu_flags = cpuinfo.get_cpu_info()['flags']
        return 'sse2' in cpu_flags, 'avx2' in cpu_flags


# Plugins

class Build(build):
    """Build command with extra options used by PluginBuildExt"""

    user_options = [
        ('hdf5=', None, "Custom path to HDF5 (as in h5py)"),
        ('openmp=', None, "Whether or not to compile with OpenMP."
         "Default: False on Windows with Python 2.7 and macOS, True otherwise"),
        ('native=', None, "Whether to compile for the building machine or for generic support (For unix compilers only)."
         "Default: True (i.e., specific to CPU used for build)"),
        ('sse2=', None, "Whether or not to compile with SSE2 support if available."
         "Default: True"),
        ('avx2=', None, "Whether or not to compile with AVX2 support if available."
         "Default: True"),
        ('cpp11=', None, "Whether or not to compile C++11 code if available."
         "Default: True"),
        ]
    user_options.extend(build.user_options)

    boolean_options = build.boolean_options + ['openmp', 'native', 'sse2', 'avx2', 'cpp11']

    def initialize_options(self):
        build.initialize_options(self)
        self.hdf5 = None
        self.openmp = not sys.platform.startswith('darwin') and (
            not sys.platform.startswith('win') or sys.version_info[0] >= 3)
        self.native = True
        self.sse2 = True
        self.avx2 = True
        self.cpp11 = True


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

        - check for OpenMP, SSE2, AVX2 availability
        - select compile args for MSVC and others
        - Set hdf5 directory
        """
        build_cmd = self.distribution.get_command_obj("build")
        compiler_type = self.compiler.compiler_type

        # Check availability of compile flags

        if build_cmd.cpp11:
            if compiler_type == 'msvc':
                with_cpp11 = sys.version_info[:2] >= (3, 5)
            else:
                with_cpp11 = self.__check_compile_flag('-std=c++11', extension='.cpp')
        else:
            with_cpp11 = False

        if build_cmd.sse2:
            if compiler_type == 'msvc':
                with_sse2 = sys.version_info[0] >= 3
            else:
                with_sse2 = self.__check_compile_flag('-msse2')
        else:
            with_sse2 = False

        if build_cmd.avx2:
            if compiler_type == 'msvc':
                with_avx2 = sys.version_info[:2] >= (3, 5)
            else:
                with_avx2 = self.__check_compile_flag('-mavx2')
        else:
            with_avx2 = False

        with_openmp = bool(build_cmd.openmp) and self.__check_compile_flag(
            '/openmp' if compiler_type == 'msvc' else '-fopenmp')

        if build_cmd.native:
            is_cpu_sse2, is_cpu_avx2 = get_cpu_sse2_avx2()
            with_sse2 = with_sse2 and is_cpu_sse2
            with_avx2 = with_avx2 and is_cpu_avx2

        logger.info("Building with C++11: %r", with_cpp11)
        logger.info('Building with native option: %r', bool(build_cmd.native))
        logger.info("Building extensions with SSE2: %r", with_sse2)
        logger.info("Building extensions with AVX2: %r", with_avx2)
        logger.info("Building extensions with OpenMP: %r", with_openmp)

        prefix = '/' if compiler_type == 'msvc' else '-'

        for e in self.extensions:
            if isinstance(e, HDF5PluginExtension):
                e.set_hdf5_dir(build_cmd.hdf5)

                if with_cpp11:
                    for name, value in e.cpp11.items():
                        attribute = getattr(e, name)
                        attribute += value

                # Enable SSE2/AVX2 if available and add corresponding resources
                if with_sse2:
                    e.extra_compile_args += ['-msse2'] # /arch:SSE2 is on by default
                    for name, value in e.sse2.items():
                        attribute = getattr(e, name)
                        attribute += value

                if with_avx2:
                    e.extra_compile_args += ['-mavx2', '/arch:AVX2']
                    for name, value in e.avx2.items():
                        attribute = getattr(e, name)
                        attribute += value

            if not with_openmp:  # Remove OpenMP flags
                e.extra_compile_args = [
                    arg for arg in e.extra_compile_args if not arg.endswith('openmp')]
                e.extra_link_args = [
                    arg for arg in e.extra_link_args if not arg.endswith('openmp')]

            if build_cmd.native:  # Add -march=native
                e.extra_compile_args += ['-march=native']

            # Remove flags that do not correspond to compiler
            e.extra_compile_args = [
                arg for arg in e.extra_compile_args if arg.startswith(prefix)]
            e.extra_link_args = [
                arg for arg in e.extra_link_args if arg.startswith(prefix)]

        build_ext.build_extensions(self)

    def __check_compile_flag(self, flag, extension='.c'):
        """Try to compile an empty file to check for compiler args

        :param str flag: Flag argument to pass to compiler
        :param str extension: Source file extension (default: '.c')
        :returns: Whether or not compilation was successful
        :rtype: bool
        """
        if sys.version_info[0] < 3:
            return False  # Not implemented for Python 2.7

        with tempfile.TemporaryDirectory() as tmp_dir:
            # Create empty source file
            tmp_file = os.path.join(tmp_dir, 'source' + extension)
            with open(tmp_file, 'w') as f:
                f.write('int main (int argc, char **argv) { return 0; }\n')

            try:
                self.compiler.compile([tmp_file], output_dir=tmp_dir, extra_postargs=[flag])
            except CompileError:
                return False
            else:
                return True


class HDF5PluginExtension(Extension):
    """Extension adding specific things to build a HDF5 plugin"""

    def __init__(self, name, sse2=None, avx2=None, cpp11=None, **kwargs):
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

        self.sse2 = sse2 if sse2 is not None else {}
        self.avx2 = avx2 if avx2 is not None else {}
        self.cpp11 = cpp11 if cpp11 is not None else {}

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
bithsuffle_dir = 'src/bitshuffle'

# Set compile args for both MSVC and others, list is stripped at build time
extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
extra_link_args = ['-fopenmp', '/openmp']

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
    extra_link_args=extra_link_args,
    )


# blosc plugin
# Plugin from https://github.com/Blosc/hdf5-blosc
# c-blosc from https://github.com/Blosc/c-blosc
hdf5_blosc_dir = 'src/hdf5-blosc/src/'
blosc_dir = 'src/c-blosc/'

# blosc sources
sources = [f for f in glob(blosc_dir + 'blosc/*.c')
           if 'avx2' not in f and 'sse2' not in f]
depends = [f for f in glob(blosc_dir + 'blosc/*.h')]
include_dirs = [blosc_dir, blosc_dir + 'blosc']
define_macros = []

sse2_kwargs = {
    'sources': [f for f in glob(blosc_dir + 'blosc/*.c') if 'sse2' in f],
    'define_macros': [('SHUFFLE_SSE2_ENABLED', 1)],
    }

avx2_kwargs = {
    'sources': [f for f in glob(blosc_dir + 'blosc/*.c') if 'avx2' in f],
    'define_macros': [('SHUFFLE_AVX2_ENABLED', 1)],
    }

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
cpp11_kwargs = {
    'sources': glob(blosc_dir + 'internal-complibs/snappy*/*.cc'),
    'include_dirs': glob(blosc_dir + 'internal-complibs/snappy*'),
    'extra_compile_args': ['-std=c++11', '-lstdc++'],
    'define_macros': [('HAVE_SNAPPY', 1)],
    }

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
    sse2=sse2_kwargs,
    avx2=avx2_kwargs,
    cpp11=cpp11_kwargs,
    )


# lz4 plugin
# Source from https://github.com/nexusformat/HDF5-External-Filter-Plugins
lz4_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5lz4",
    sources=['src/LZ4/H5Zlz4.c'] + \
            lz4_sources,
    depends=lz4_depends,
    include_dirs=lz4_include_dirs,
    libraries=['Ws2_32'] if sys.platform.startswith('win') else [],
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

