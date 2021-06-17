# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2021 European Synchrotron Radiation Facility
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
__date__ = "15/12/2020"


from glob import glob
import logging
import os
import sys
import tempfile
import platform
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.sdist import sdist
from setuptools.command.build_py import build_py
from distutils.command.build import build
from distutils import ccompiler, errors, sysconfig


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


# Patch bdist_wheel
try:
    from wheel.bdist_wheel import bdist_wheel
except ImportError:
    BDistWheel = None
else:
    from pkg_resources import parse_version
    import wheel
    from wheel.bdist_wheel import get_platform

    class BDistWheel(bdist_wheel):
        """Override bdist_wheel to handle as pure python package"""

        def finalize_options(self):
            if parse_version(wheel.__version__) >= parse_version('0.34.0'):
                self.plat_name = get_platform(self.bdist_dir)
            else:
                self.plat_name = get_platform()
            bdist_wheel.finalize_options(self)

        def get_tag(self):
            """Override the python and abi tag generation"""
            return self.python_tag, "none", bdist_wheel.get_tag(self)[-1]


def get_cpu_sse2_avx2():
    """Returns whether SSE2 and AVX2 are available on the current CPU

    :returns: (is SSE2 available, is AVX2 available)
    :rtype: List(bool)
    """
    if platform.machine() in ["arm64", "mips64"]:
        return False, False
    if platform.machine() == "ppc64le":
        return True, False
    try:
        import cpuinfo
    except ImportError as e:
        raise e
    except Exception:  # cpuinfo raises Exception for unsupported architectures
        logger.warning(
            "CPU info detection does not support this architecture: SSE2 and AVX2 disabled")
        return False, False
    else:
        cpu_flags = cpuinfo.get_cpu_info().get('flags', [])
        return 'sse2' in cpu_flags, 'avx2' in cpu_flags


# Plugins

def check_compile_flag(compiler, flag, extension='.c'):
    """Try to compile an empty file to check for compiler args

    :param distutils.ccompiler.CCompiler compiler: The compiler to use
    :param str flag: Flag argument to pass to compiler
    :param str extension: Source file extension (default: '.c')
    :returns: Whether or not compilation was successful
    :rtype: bool
    """
    with tempfile.TemporaryDirectory() as tmp_dir:
        # Create empty source file
        tmp_file = os.path.join(tmp_dir, 'source' + extension)
        with open(tmp_file, 'w') as f:
            f.write('int main (int argc, char **argv) { return 0; }\n')

        try:
            compiler.compile([tmp_file], output_dir=tmp_dir, extra_postargs=[flag])
        except errors.CompileError:
            return False
        else:
            return True


class Build(build):
    """Build command with extra options used by PluginBuildExt"""

    user_options = [
        ('hdf5=', None, "Custom path to HDF5 (as in h5py)"),
        ('openmp=', None, "Whether or not to compile with OpenMP."
         "Default: False on macOS, True otherwise"),
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
        self.hdf5 = os.environ.get("HDF5PLUGIN_HDF5_DIR", None)
        self.openmp = os.environ.get(
            "HDF5PLUGIN_OPENMP",
            "False" if sys.platform.startswith('darwin') else "True"
        ) == "True"
        self.native = os.environ.get("HDF5PLUGIN_NATIVE", "True") == "True"
        self.sse2 = os.environ.get("HDF5PLUGIN_SSE2", "True") == "True"
        self.avx2 = os.environ.get("HDF5PLUGIN_AVX2", "True") == "True"
        self.cpp11 = os.environ.get("HDF5PLUGIN_CPP11", "True") == "True"

    def finalize_options(self):
        build.finalize_options(self)

        # Check that build options are available
        compiler = ccompiler.new_compiler(compiler=self.compiler, force=True)
        sysconfig.customize_compiler(compiler)

        if self.cpp11:
            if compiler.compiler_type == 'msvc':
                self.cpp11 = sys.version_info[:2] >= (3, 5)
            else:
                self.cpp1 = check_compile_flag(
                    compiler, '-std=c++11', extension='.cc')
            if not self.cpp11:
                logger.warning("C++11 disabled: not available")

        if self.sse2:
            if platform.machine() == 'arm64':
                self.sse2 = False
            elif (compiler.compiler_type == 'msvc' or
                    platform.machine() == 'ppc64le'):
                self.sse2 = True
            else:
                self.sse2 = check_compile_flag(compiler, '-msse2')
            if not self.sse2:
                logger.warning("SSE2 disabled: not available")

        if self.avx2:
            if platform.machine() == 'arm64':
                self.avx2 = False
            elif compiler.compiler_type == 'msvc':
                self.avx2 = sys.version_info[:2] >= (3, 5)
            elif platform.machine() == 'ppc64le':
                self.avx2 = False
            else:
                self.avx2 = check_compile_flag(compiler, '-mavx2')
            if not self.avx2:
                logger.warning("AVX2 disabled: not available")

        if self.openmp:
            prefix = '/' if compiler.compiler_type == 'msvc' else '-f'
            self.openmp = check_compile_flag(compiler, prefix + 'openmp')
            if not self.openmp:
                logger.warning("OpenMP disabled: not available")

        if self.native:
            is_cpu_sse2, is_cpu_avx2 = get_cpu_sse2_avx2()
            self.sse2 = self.sse2 and is_cpu_sse2
            self.avx2 = self.avx2 and is_cpu_avx2

        logger.info("Building with C++11: %r", bool(self.cpp11))
        logger.info('Building with native option: %r', bool(self.native))
        logger.info("Building with SSE2: %r", bool(self.sse2))
        logger.info("Building with AVX2: %r", bool(self.avx2))
        logger.info("Building with OpenMP: %r", bool(self.openmp))

        # Filter out C++11 libraries if cpp11 option is False
        self.distribution.libraries = [
            (name, info) for name, info in self.distribution.libraries
            if self.cpp11 or '-std=c++11' not in info.get('cflags', [])]

        # Filter out C++11-only extensions if cpp11 option is False
        self.distribution.ext_modules = [
            ext for ext in self.distribution.ext_modules
            if self.cpp11 or not (isinstance(ext, HDF5PluginExtension) and ext.cpp11_required)]

        # Generate config file content
        build_config = {
            'openmp': bool(self.openmp),
            'native': bool(self.native),
            'sse2': bool(self.sse2),
            'avx2': bool(self.avx2),
            'cpp11': bool(self.cpp11),
        }
        self.hdf5plugin_config_str = 'config = ' + str(build_config) + '\n'
        self.hdf5plugin_config_file = os.path.join(
            self.build_lib, PROJECT, '_config.py')

    def has_config_changed(self):
        """Check if configuration file has changed"""
        try:
            with open(self.hdf5plugin_config_file, 'r') as f:
                if f.read() == self.hdf5plugin_config_str:
                    logger.info("Build configuration didn't changed")
                    return False  # Configuration is the same
        except:
            pass
        logger.info('Build configuration has changed')
        clean_cmd = self.distribution.get_command_obj('clean')
        clean_cmd.all = True
        return True

    # Add clean to sub-commands
    sub_commands = [('clean', has_config_changed)] + build.sub_commands


class BuildPy(build_py):
    """build_py command also writing hdf5plugin._config"""
    def run(self):
        super().run()

        build_cmd = self.distribution.get_command_obj("build")

        # Save config to file
        with open(build_cmd.hdf5plugin_config_file, 'w') as f:
            f.write(build_cmd.hdf5plugin_config_str)


class PluginBuildExt(build_ext):
    """Build extension command for DLLs that are not Python modules

    It also handles extra compile arguments depending on the build options.
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
        prefix = '/' if self.compiler.compiler_type == 'msvc' else '-'

        for e in self.extensions:
            if isinstance(e, HDF5PluginExtension):
                e.set_hdf5_dir(build_cmd.hdf5)

                if build_cmd.cpp11:
                    for name, value in e.cpp11.items():
                        attribute = getattr(e, name)
                        attribute += value

                # Enable SSE2/AVX2 if available and add corresponding resources
                if build_cmd.sse2:
                    if platform.machine() == 'ppc64le':
                        # Power9 way of enabling SSE2 support
                        e.extra_compile_args += ['-DNO_WARN_X86_INTRINSICS']
                    else:
                        e.extra_compile_args += ['-msse2'] # /arch:SSE2 is on by default
                    for name, value in e.sse2.items():
                        attribute = getattr(e, name)
                        attribute += value

                if build_cmd.avx2:
                    e.extra_compile_args += ['-mavx2', '/arch:AVX2']
                    for name, value in e.avx2.items():
                        attribute = getattr(e, name)
                        attribute += value

            if not build_cmd.openmp:  # Remove OpenMP flags
                e.extra_compile_args = [
                    arg for arg in e.extra_compile_args if not arg.endswith('openmp')]
                e.extra_link_args = [
                    arg for arg in e.extra_link_args if not arg.endswith('openmp')]

            if build_cmd.native:  # Add -march=native
                if platform.machine() in ["i386", "i686", "amd64", "x86_64", "mips64"]:
                    e.extra_compile_args += ['-march=native']
                else:
                    e.extra_compile_args += ['-mcpu=native']

            # Remove flags that do not correspond to compiler
            e.extra_compile_args = [
                arg for arg in e.extra_compile_args if arg.startswith(prefix)]
            e.extra_link_args = [
                arg for arg in e.extra_link_args if arg.startswith(prefix)]

        build_ext.build_extensions(self)


class HDF5PluginExtension(Extension):
    """Extension adding specific things to build a HDF5 plugin"""

    def __init__(self, name, sse2=None, avx2=None, cpp11=None, cpp11_required=False, **kwargs):
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
        self.cpp11_required = cpp11_required

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
                folder = 'windows'
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
if platform.machine() == "ppc64le":
    # Required on ppc64le
    sse2_options = {'extra_compile_args': ['-DUSESSE2'] }
else:
    sse2_options = {}
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
    sse2=sse2_options,
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

if platform.machine() == 'ppc64le':
    # SSE2 support in blosc uses x86 assembly code in shuffle
    sse2_kwargs = {}
else:
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
snappy_dir = 'src/snappy-1.1.1/'
snappy_lib = ('snappy', {
    'sources': glob(snappy_dir + '*.cc'),
    'include_dirs': glob(snappy_dir),
    'cflags': ['-std=c++11']})

cpp11_kwargs = {
    'include_dirs': glob(snappy_dir),
    'extra_link_args': ['-lstdc++'],
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

extra_compile_args = ['-std=gnu99']  # Needed to build manylinux1 wheels

blosc_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5blosc",
    sources=sources + \
        prefix(hdf5_blosc_dir,['blosc_filter.c', 'blosc_plugin.c']),
    depends=depends + \
        prefix(hdf5_blosc_dir, ['blosc_filter.h', 'blosc_plugin.h']),
    include_dirs=include_dirs + [hdf5_blosc_dir],
    define_macros=define_macros,
    extra_compile_args=extra_compile_args,
    sse2=sse2_kwargs,
    avx2=avx2_kwargs,
    cpp11=cpp11_kwargs,
    )

# HDF5Plugin-Zstandard
zstandard_dir = os.path.join("src", "HDF5Plugin-Zstandard")
zstandard_include_dirs = glob(blosc_dir + 'internal-complibs/zstd*')
zstandard_include_dirs += glob(blosc_dir + 'internal-complibs/zstd*/common')
zstandard_sources = [os.path.join(zstandard_dir, 'zstd_h5plugin.c')]
zstandard_sources += glob(blosc_dir +'internal-complibs/zstd*/*/*.c')
zstandard_depends = [os.path.join(zstandard_dir, 'zstd_h5plugin.h')]
zstandard_depends += glob(blosc_dir +'internal-complibs/zstd*/*/*.h')
zstandard_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5zstd",
    sources=zstandard_sources,
    depends=zstandard_depends,
    include_dirs=zstandard_include_dirs,
    define_macros=define_macros,
    )


# lz4 plugin
# Source from https://github.com/nexusformat/HDF5-External-Filter-Plugins
if sys.platform.startswith('darwin'):
    extra_compile_args = ['-Wno-error=implicit-function-declaration']
else:
    extra_compile_args = []

lz4_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5lz4",
    sources=['src/LZ4/H5Zlz4.c', 'src/lz4-r122/lz4.c'],
    depends=['src/lz4-r122/lz4.h'],
    include_dirs=['src/lz4-r122'],
    extra_compile_args=extra_compile_args,
    libraries=['Ws2_32'] if sys.platform.startswith('win') else [],
    )


# FCIDECOMP
fcidecomp_dir = 'src/fcidecomp/FCIDECOMP_V1.0.2/Software/FCIDECOMP_SOURCES'
extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
extra_link_args = ['-fopenmp', '/openmp']
fcidecomp_additional_dirs = ["fcicomp-common",
                             "fcicomp-H5Zjpegls",
                             "fcicomp-jpegls",
                             ]
sources = []
depends = []
include_dirs = []
for item in fcidecomp_additional_dirs:
    sources += glob(fcidecomp_dir + "/" + item + "/src/*.c")
    depends += glob(fcidecomp_dir + "/" + item + "/include/*.h")
    include_dirs += [fcidecomp_dir + "/" + item + "/include",
                     "src/charls/src"]
    #include_dirs += ["src/hdf5"]
cpp11_kwargs = {
    'extra_link_args': ['-lstdc++'],
    }
fcidecomp_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5fcidecomp",
    sources=sources,
    depends=depends,
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    # export_symbols=['init_filter'],
    cpp11=cpp11_kwargs,
    cpp11_required=True,
    define_macros=[('CHARLS_STATIC', 1)],
    )

# CharLS
charls_dir = "src/charls/src"
charls_sources = glob(charls_dir + '/*.cpp')
charls_include_dirs = [charls_dir]
charls_lib = ('charls', {
    'sources': charls_sources,
    'include_dirs': charls_include_dirs,
    'cflags': ['-std=c++11']})

cpp11_kwargs = {
    'include_dirs': glob('charls_dir/src'),
    'extra_link_args': ['-lstdc++'],
    }

# H5Z-ZFP
h5zfp_dir = 'src/H5Z-ZFP/src'
extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
extra_link_args = ['-fopenmp', '/openmp']

sources = glob(h5zfp_dir + "/" + "*.c")
depends = glob(h5zfp_dir + "/" + "*.h")
include_dirs = [h5zfp_dir + "/src", "src/zfp/include"]
h5zfp_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5zfp",
    sources=sources,
    depends=depends,
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    )

# zfp
zfp_dir = os.path.join("src", "zfp")
zfp_sources = glob(os.path.join(zfp_dir, 'src', '*.c'))
zfp_include_dirs = [os.path.join(zfp_dir, 'include')]
zfp_lib = ('zfp', {
    'sources': zfp_sources,
    'include_dirs': zfp_include_dirs,
    'cflags': ['-DBIT_STREAM_WORD_TYPE=uint8'],
    })

libraries = [snappy_lib, charls_lib, zfp_lib]

extensions = [lz4_plugin,
              bithsuffle_plugin,
              blosc_plugin,
              fcidecomp_plugin,
              h5zfp_plugin,
              zstandard_plugin,
              ]


# setup

def get_version(debian=False):
    """Returns current version number from _version.py file"""
    dirname = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "src", PROJECT)
    sys.path.insert(0, dirname)
    dont_write_bytecode = sys.dont_write_bytecode
    sys.dont_write_bytecode = True  # Avoid creating __pycache__/_version.pyc
    import _version
    sys.path = sys.path[1:]
    sys.dont_write_bytecode = dont_write_bytecode
    return _version.debianversion if debian else _version.strictversion


################################################################################
# Debian source tree
################################################################################

class sdist_debian(sdist):
    """
    Tailor made sdist for debian
    * remove auto-generated doc
    * remove cython generated .c files
    * remove cython generated .cpp files
    * remove .bat files
    * include .l man files
    """

    description = "Create a source distribution for Debian (tarball, zip file, etc.)"

    @staticmethod
    def get_debian_name():
        name = "%s_%s" % (PROJECT, get_version(debian=True))
        return name

    def prune_file_list(self):
        sdist.prune_file_list(self)
        to_remove = ["doc/build", "doc/pdf", "doc/html", "pylint", "epydoc"]
        print("Removing files for debian")
        for rm in to_remove:
            self.filelist.exclude_pattern(pattern="*", anchor=False, prefix=rm)

        # this is for Cython files specifically: remove C & html files
        search_root = os.path.dirname(os.path.abspath(__file__))
        for root, _, files in os.walk(search_root):
            for afile in files:
                if os.path.splitext(afile)[1].lower() == ".pyx":
                    base_file = os.path.join(root, afile)[len(search_root) + 1:-4]
                    self.filelist.exclude_pattern(pattern=base_file + ".c")
                    self.filelist.exclude_pattern(pattern=base_file + ".cpp")
                    self.filelist.exclude_pattern(pattern=base_file + ".html")

        # do not include third_party/_local files
        self.filelist.exclude_pattern(pattern="*", prefix="silx/third_party/_local")

    def make_distribution(self):
        self.prune_file_list()
        sdist.make_distribution(self)
        dest = self.archive_files[0]
        dirname, basename = os.path.split(dest)
        base, ext = os.path.splitext(basename)
        while ext in [".zip", ".tar", ".bz2", ".gz", ".Z", ".lz", ".orig"]:
            base, ext = os.path.splitext(base)
        # if ext:
        #     dest = "".join((base, ext))
        # else:
        #     dest = base
        # sp = dest.split("-")
        # base = sp[:-1]
        # nr = sp[-1]
        debian_arch = os.path.join(dirname, self.get_debian_name() + ".orig.tar.gz")
        os.rename(self.archive_files[0], debian_arch)
        self.archive_files = [debian_arch]
        print("Building debian .orig.tar.gz in %s" % self.archive_files[0])


PROJECT = 'hdf5plugin'
author = "ESRF - Data Analysis Unit"
author_email = "silx@esrf.fr"
description = "HDF5 Plugins for Windows, MacOS, and Linux"
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
               "Programming Language :: Python :: 3.4",
               "Programming Language :: Python :: 3.5",
               "Programming Language :: Python :: 3.6",
               "Programming Language :: Python :: 3.7",
               "Programming Language :: Python :: 3.8",
               "Programming Language :: Python :: 3.9",
               "Topic :: Software Development :: Libraries :: Python Modules",
               ]
cmdclass = dict(build=Build,
                build_ext=PluginBuildExt,
                build_py=BuildPy,
                debian_src=sdist_debian)
if BDistWheel is not None:
    cmdclass['bdist_wheel'] = BDistWheel


if __name__ == "__main__":
    setup(name=PROJECT,
          version=get_version(),
          author=author,
          author_email=author_email,
          url=url,
          python_requires='>=3.4',
          classifiers=classifiers,
          description=description,
          long_description=long_description,
          license=license,
          packages=[PROJECT],
          package_dir={'': 'src'},
          ext_modules=extensions,
          install_requires=['h5py'],
          setup_requires=['setuptools'],
          cmdclass=cmdclass,
          libraries=libraries,
          zip_safe=False,
          )
