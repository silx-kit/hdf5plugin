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
from setuptools.command.build_clib import build_clib
from distutils.command.build import build
from distutils import ccompiler, errors, sysconfig

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    import cpuinfo
except ImportError as e:
    raise e
except Exception:  # cpuinfo raises Exception for unsupported architectures
    logger.warning("Architecture is not supported by cpuinfo")
    cpuinfo = None


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


# Probe host capabilities and manage build config

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


def has_cpu_flag(flag: str) -> bool:
    """Is given flag available on the current (x86) CPU"""
    if cpuinfo is None:
        logger.warning("Cannot get CPU flags")
        return False
    return flag in cpuinfo.get_cpu_info().get('flags', [])


class HostConfig:
    """Probe and describe host configuration

    For differences in native build option across architectures, see
    https://gcc.gnu.org/onlinedocs/gcc/Submodel-Options.html#Submodel-Options
    """
    def __init__(self, compiler=None):
        compiler = ccompiler.new_compiler(compiler, force=True)
        sysconfig.customize_compiler(compiler)
        self.__compiler = compiler

        # Get machine architecture description
        self.machine = platform.machine().lower()
        if cpuinfo is not None:
            self.arch = cpuinfo._parse_arch(self.machine)[0]
        else:
            self.arch = None

        # Set architecture specific compile args
        if self.arch in ('X86_32', 'X86_64', 'MIPS_64'):
            self.native_compile_args = ("-march=native",)
        elif self.arch in ('ARM_7', 'ARM_8', 'PPC_64'):
            self.native_compile_args = ("-mcpu=native",)
        else:
            self.native_compile_args = ()

        if self.arch in ('X86_32', 'X86_64'):
            self.sse2_compile_args = ('-msse2',)  # /arch:SSE2 is on by default
        elif self.machine == 'ppc64le':
            self.sse2_compile_args = ('-DNO_WARN_X86_INTRINSICS',)  # P9 way to enable SSE2
        else:
            self.sse2_compile_args = ()

        if self.arch in ('X86_32', 'X86_64'):
            self.avx2_compile_args = ('-mavx2', '/arch:AVX2')
        else:
            self.avx2_compile_args = ()

    def get_shared_lib_extension(self) -> str:
        """Returns shared library file extension"""
        if sys.platform.startswith('win'):
            return '.dll'
        elif sys.platform.startswith('linux'):
            return '.so'
        elif sys.platform.startswith('darwin'):
            return '.dylib'
        else:  # Return same value as used by build_ext.get_ext_filename
            return sysconfig.get_config_var('EXT_SUFFIX')

    def has_cpp11(self) -> bool:
        """Check C++11 availability on host"""
        if self.__compiler.compiler_type == 'msvc':
            return sys.version_info[:2] >= (3, 5)
        return check_compile_flag(self.__compiler, '-std=c++11', extension='.cc')

    def has_sse2(self) -> bool:
        """Check SSE2 availability on host"""
        if self.arch in ('X86_32', 'X86_64'):
            if not has_cpu_flag('sse2'):
                return False  # SSE2 not available on host
            return (self.__compiler.compiler_type == 'msvc' or
                    check_compile_flag(self.__compiler, '-msse2'))
        if self.machine == 'ppc64le':
            return True
        return False  # Disabled by default

    def has_avx2(self) -> bool:
        """Check AVX2 availability on host"""
        if self.arch in ('X86_32', 'X86_64'):
            if not has_cpu_flag('avx2'):
                return False  # AVX2 not available on host
            if self.__compiler.compiler_type == 'msvc':
                return sys.version_info[:2] >= (3, 5)
            return check_compile_flag(self.__compiler, '-mavx2')
        return False  # Disabled by default

    def has_openmp(self) -> bool:
        """Check OpenMP availability on host"""
        if sys.platform.startswith('darwin'):
            return False
        prefix = '/' if self.__compiler.compiler_type == 'msvc' else '-f'
        return check_compile_flag(self.__compiler, prefix + 'openmp')

    def has_native(self) -> bool:
        """Returns native build option availability on host"""
        return len(self.native_compile_args) > 0


class BuildConfig:
    """Describe build configuration"""
    def __init__(
            self,
            config_file: str,
            compiler=None,
            hdf5_dir=None,
            use_cpp11=None,
            use_sse2=None,
            use_avx2=None,
            use_openmp=None,
            use_native=None,
        ):

        self.__config_file = str(config_file)

        host_config = HostConfig(compiler)
        self.__filter_file_extension = host_config.get_shared_lib_extension()

        # Build option priority order: 1. command line, 2. env. var., 3. probed values
        if hdf5_dir is None:
            hdf5_dir = os.environ.get("HDF5PLUGIN_HDF5_DIR", None)
        self.__hdf5_dir = hdf5_dir

        if use_cpp11 is None:
            env_cpp11 = os.environ.get("HDF5PLUGIN_CPP11", None)
            use_cpp11 = host_config.has_cpp11() if env_cpp11 is None else env_cpp11 == "True"
        self.__use_cpp11 = bool(use_cpp11)

        if use_sse2 is None:
            env_sse2 = os.environ.get("HDF5PLUGIN_SSE2", None)
            use_sse2 = host_config.has_sse2() if env_sse2 is None else env_sse2 == "True"
        if use_avx2 is None:
            env_avx2 = os.environ.get("HDF5PLUGIN_AVX2", None)
            use_avx2 = host_config.has_avx2() if env_avx2 is None else env_avx2 == "True"
        if use_avx2 and not use_sse2:
            logger.error(
                "use_avx2=True disabled: incompatible with use_sse2=False")
            use_avx2 = False
        self.__use_sse2 = bool(use_sse2)
        self.__use_avx2 = bool(use_avx2)

        if use_openmp is None:
            env_openmp = os.environ.get("HDF5PLUGIN_OPENMP", None)
            use_openmp = host_config.has_openmp() if env_openmp is None else env_openmp == "True"
        self.__use_openmp = bool(use_openmp)

        if use_native is None:
            env_native = os.environ.get("HDF5PLUGIN_NATIVE", None)
            use_native = host_config.has_native() if env_native is None else env_native == "True"
        self.__use_native = bool(use_native)

        # Gather used compile args
        compile_args = []
        if self.__use_sse2:
            compile_args.extend(host_config.sse2_compile_args)
        if self.__use_avx2:
            compile_args.extend(host_config.avx2_compile_args)
        if self.__use_native:
            compile_args.extend(host_config.native_compile_args)
        self.__compile_args = tuple(compile_args)

    hdf5_dir = property(lambda self: self.__hdf5_dir)
    filter_file_extension = property(lambda self: self.__filter_file_extension)
    use_cpp11 = property(lambda self: self.__use_cpp11)
    use_sse2 = property(lambda self: self.__use_sse2)
    use_avx2 = property(lambda self: self.__use_avx2)
    use_openmp = property(lambda self: self.__use_openmp)
    use_native = property(lambda self: self.__use_native)
    compile_args = property(lambda self: self.__compile_args)

    def get_config_string(self):
        build_config = {
            'openmp': self.use_openmp,
            'native': self.use_native,
            'sse2': self.use_sse2,
            'avx2': self.use_avx2,
            'cpp11': self.use_cpp11,
            'filter_file_extension': self.filter_file_extension,
        }
        return 'config = ' + str(build_config) + '\n'

    def has_config_changed(self) -> bool:
        """Returns whether config file needs to be changed or not."""
        try:
            with open(self.__config_file, 'r') as f:
                return f.read() != self.get_config_string()
        except:
            pass
        return True

    def save_config(self) -> None:
        """Save config as a dict in a python file"""
        with open(self.__config_file, 'w') as f:
            f.write(self.get_config_string())


# Plugins

class Build(build):
    """Build command with extra options used by PluginBuildExt"""

    user_options = [
        ('hdf5=', None, "Custom path to HDF5 (as in h5py). "
         "Default: HDF5PLUGIN_HDF5_DIR environment variable if set."),
        ('openmp=', None, "Whether or not to compile with OpenMP. "
         "Default: HDF5PLUGIN_OPENMP env. var. if set, else "
         "True if probed (always False on macOS)."),
        ('native=', None, "True to compile specifically for the host, "
         "False for generic support (For unix compilers only). "
         "Default: HDF5PLUGIN_NATIVE env. var. if set, else "
         "True on supported architectures, False otherwise"),
        ('sse2=', None, "Whether or not to compile with SSE2 support. "
         "Default: HDF5PLUGIN_SSE2 env. var. if set, else "
         "True on ppc64le and when probed on x86, False otherwise"),
        ('avx2=', None, "Whether or not to compile with AVX2 support. "
         "avx2=True requires sse2=True. "
         "Default: HDF5PLUGIN_AVX2 env. var. if set, else "
         "True on x86 when probed, False otherwise"),
        ('cpp11=', None, "Whether or not to compile C++11 code if available."
         "Default: HDF5PLUGIN_CPP11 env. var. if set, else "
         "True if probed."),
        ]
    user_options.extend(build.user_options)

    boolean_options = build.boolean_options + ['openmp', 'native', 'sse2', 'avx2', 'cpp11']

    def initialize_options(self):
        build.initialize_options(self)
        self.hdf5 = None
        self.cpp11 = None
        self.openmp = None
        self.native = None
        self.sse2 = None
        self.avx2 = None

    def finalize_options(self):
        build.finalize_options(self)
        self.hdf5plugin_config = BuildConfig(
            config_file=os.path.join(self.build_lib, PROJECT, '_config.py'),
            compiler=self.compiler,
            hdf5_dir=self.hdf5,
            use_cpp11=self.cpp11,
            use_sse2=self.sse2,
            use_avx2=self.avx2,
            use_openmp=self.openmp,
            use_native=self.native,
        )
        logger.info("Build configuration: %s", self.hdf5plugin_config.get_config_string())

        if not self.hdf5plugin_config.use_cpp11:
            # Filter out C++11 libraries
            self.distribution.libraries = [
                (name, info) for name, info in self.distribution.libraries
                if '-std=c++11' not in info.get('cflags', [])]

            # Filter out C++11-only extensions
            self.distribution.ext_modules = [
                ext for ext in self.distribution.ext_modules
                if not (isinstance(ext, HDF5PluginExtension) and ext.cpp11_required)]

    def has_config_changed(self):
        """Check if configuration file has changed"""
        if not self.hdf5plugin_config.has_config_changed():
            logger.info("Build configuration didn't changed")
            return False

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
        build_cmd.hdf5plugin_config.save_config()


class BuildCLib(build_clib):
    """build_clib command adding/patching compile args"""
    def build_libraries(self, libraries):
        updated_libraries = []
        for (lib_name, build_info) in libraries:
            cflags = list(build_info.get('cflags', []))

            # Add flags from build config that corresponds to the compiler
            config = self.distribution.get_command_obj("build").hdf5plugin_config
            prefix = '/' if self.compiler.compiler_type == 'msvc' else '-'
            cflags.extend(
                [f for f in config.compile_args if f.startswith(prefix)])

            build_info['cflags'] = cflags

            updated_libraries.append((lib_name, build_info))

        super().build_libraries(updated_libraries)


class PluginBuildExt(build_ext):
    """Build extension command for DLLs that are not Python modules

    It also handles extra compile arguments depending on the build options.
    """

    def get_export_symbols(self, ext):
        """Overridden to remove PyInit_* export"""
        return ext.export_symbols

    def get_ext_filename(self, ext_name):
        """Overridden to use .dll as file extension"""
        config = self.distribution.get_command_obj("build").hdf5plugin_config
        return os.path.join(*ext_name.split('.')) + config.filter_file_extension

    def build_extensions(self):
        """Overridden to tune extensions.

        - check for OpenMP, SSE2, AVX2 availability
        - select compile args for MSVC and others
        - Set hdf5 directory
        """
        config = self.distribution.get_command_obj("build").hdf5plugin_config

        for e in self.extensions:
            if isinstance(e, HDF5PluginExtension):
                e.set_hdf5_dir(config.hdf5_dir)
                e.extra_compile_args.extend(config.compile_args)

                if config.use_cpp11:
                    for name, value in e.cpp11.items():
                        attribute = getattr(e, name)
                        attribute += value
                if config.use_sse2:
                    for name, value in e.sse2.items():
                        attribute = getattr(e, name)
                        attribute += value
                if config.use_avx2:
                    for name, value in e.avx2.items():
                        attribute = getattr(e, name)
                        attribute += value

            if not config.use_openmp:  # Remove OpenMP flags
                e.extra_compile_args = [
                    arg for arg in e.extra_compile_args if not arg.endswith('openmp')]
                e.extra_link_args = [
                    arg for arg in e.extra_link_args if not arg.endswith('openmp')]

            # Remove flags that do not correspond to compiler
            prefix = '/' if self.compiler.compiler_type == 'msvc' else '-'
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
snappy_dir = 'src/snappy/'
snappy_lib = ('snappy', {
    'sources': [
        snappy_dir + 'snappy-c.cc',
        snappy_dir + 'snappy-sinksource.cc',
        snappy_dir + 'snappy-stubs-internal.cc',
        snappy_dir + 'snappy.cc',
    ],
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
extra_compile_args += ['-pthread']
extra_link_args = ['-pthread']

blosc_plugin = HDF5PluginExtension(
    "hdf5plugin.plugins.libh5blosc",
    sources=sources + \
        prefix(hdf5_blosc_dir,['blosc_filter.c', 'blosc_plugin.c']),
    depends=depends + \
        prefix(hdf5_blosc_dir, ['blosc_filter.h', 'blosc_plugin.h']),
    include_dirs=include_dirs + [hdf5_blosc_dir],
    define_macros=define_macros,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
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
classifiers = ["Development Status :: 5 - Production/Stable",
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
cmdclass = dict(
    build=Build,
    build_clib=BuildCLib,
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
          extras_require={'dev': ['sphinx', 'sphinx_rtd_theme']},
          cmdclass=cmdclass,
          libraries=libraries,
          zip_safe=False,
          )
