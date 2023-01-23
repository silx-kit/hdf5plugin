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
__date__ = "05/12/2022"


from glob import glob
import itertools
import logging
import os
import sys
import sysconfig
import tempfile
import platform
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.sdist import sdist
from setuptools.command.build_py import build_py
from setuptools.command.build_clib import build_clib
try:  # setuptools >= 62.4.0
    from setuptools.command.build import build
except ImportError:
    from distutils.command.build import build
try:  # setuptools >= 59.0.0
    from setuptools.errors import CompileError
except ImportError:
    from distutils.errors import CompileError
import distutils.ccompiler
import distutils.sysconfig

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    import cpuinfo
except ImportError as e:
    raise e
except Exception:  # cpuinfo raises Exception for unsupported architectures
    logger.warning("Architecture is not supported by cpuinfo")
    cpuinfo = None

try:  # Embedded copy of cpuinfo
    from cpuinfo import _parse_arch as cpuinfo_parse_arch
except Exception:
    try:  # Installed version of cpuinfo (when installing with pip)
        from cpuinfo.cpuinfo import _parse_arch as cpuinfo_parse_arch
    except Exception:
        cpuinfo_parse_arch = None

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

def check_compile_flags(compiler, *flags, extension='.c'):
    """Try to compile an empty file to check for compiler args

    :param distutils.ccompiler.CCompiler compiler: The compiler to use
    :param flags: Flags argument to pass to compiler
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
            compiler.compile([tmp_file], output_dir=tmp_dir, extra_postargs=list(flags))
        except CompileError:
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

    ARCH = cpuinfo_parse_arch(platform.machine().lower())[0] if cpuinfo_parse_arch is not None else None
    """Machine architecture description from cpuinfo parser"""

    def __init__(self, compiler=None):
        compiler = distutils.ccompiler.new_compiler(compiler, force=True)
        distutils.sysconfig.customize_compiler(compiler)
        self.__compiler = compiler

        # Set architecture specific compile args
        if self.ARCH in ('X86_32', 'X86_64', 'MIPS_64'):
            self.native_compile_args = ("-march=native",)
        elif self.ARCH in ('ARM_7', 'ARM_8', 'PPC_64'):
            self.native_compile_args = ("-mcpu=native",)
        else:
            self.native_compile_args = ()

        if self.ARCH in ('X86_32', 'X86_64'):
            self.sse2_compile_args = ('-msse2',)  # /arch:SSE2 is on by default
        else:
            self.sse2_compile_args = ()

        if self.ARCH in ('X86_32', 'X86_64'):
            self.avx2_compile_args = ('-mavx2', '/arch:AVX2')
        else:
            self.avx2_compile_args = ()

        if self.ARCH in ('X86_32', 'X86_64'):
            self.avx512_compile_args = ('-mavx512f', '-mavx512bw', '/arch:AVX512')
        else:
            self.avx512_compile_args = ()

    def get_shared_lib_extension(self) -> str:
        """Returns shared library file extension"""
        if sys.platform == 'win32':
            return '.dll'
        elif sys.platform == 'linux':
            return '.so'
        elif sys.platform == 'darwin':
            return '.dylib'
        else:  # Return same value as used by build_ext.get_ext_filename
            return sysconfig.get_config_var('EXT_SUFFIX')

    def has_cpp11(self) -> bool:
        """Check C++11 availability on host"""
        if self.__compiler.compiler_type == 'msvc':
            return True
        return check_compile_flags(self.__compiler, '-std=c++11', extension='.cc')

    def has_cpp14(self) -> bool:
        """Check C++14 availability on host"""
        if self.__compiler.compiler_type == 'msvc':
            return True
        return check_compile_flags(self.__compiler, '-std=c++14', extension='.cc')

    def has_sse2(self) -> bool:
        """Check SSE2 availability on host"""
        if self.ARCH in ('X86_32', 'X86_64'):
            if not has_cpu_flag('sse2'):
                return False  # SSE2 not available on host
            if self.__compiler.compiler_type == "msvc":
                return True
            return check_compile_flags(self.__compiler, "-msse2")
        return False  # Disabled by default

    def has_avx2(self) -> bool:
        """Check AVX2 availability on host"""
        if self.ARCH in ('X86_32', 'X86_64'):
            if not has_cpu_flag('avx2'):
                return False  # AVX2 not available on host
            if self.__compiler.compiler_type == 'msvc':
                return True
            return check_compile_flags(self.__compiler, '-mavx2')
        return False  # Disabled by default

    def has_avx512(self) -> bool:
        """Check AVX512 "F" and "BW" instruction sets availability on host"""
        if self.ARCH in ('X86_32', 'X86_64'):
            if not (has_cpu_flag('avx512f') and has_cpu_flag('avx512bw')):
                return False  # AVX512 F and/or BW not available on host
            if self.__compiler.compiler_type == 'msvc':
                return True
            return check_compile_flags(self.__compiler, '-mavx512f', '-mavx512bw')
        return False  # Disabled by default

    def has_openmp(self) -> bool:
        """Check OpenMP availability on host"""
        if sys.platform == 'darwin':
            return False
        prefix = '/' if self.__compiler.compiler_type == 'msvc' else '-f'
        return check_compile_flags(self.__compiler, prefix + 'openmp')

    def has_native(self) -> bool:
        """Returns native build option availability on host"""
        if len(self.native_compile_args) > 0:
            return check_compile_flags(self.__compiler, *self.native_compile_args)
        return False


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

        env_cpp14 = os.environ.get("HDF5PLUGIN_CPP14", None)
        use_cpp14 = host_config.has_cpp14() if env_cpp14 is None else env_cpp14 == "True"
        self.__use_cpp14 = bool(use_cpp14)

        if use_sse2 is None:
            env_sse2 = os.environ.get("HDF5PLUGIN_SSE2", None)
            use_sse2 = host_config.has_sse2() if env_sse2 is None else env_sse2 == "True"
        self.__use_sse2 = bool(use_sse2)

        if use_avx2 is None:
            env_avx2 = os.environ.get("HDF5PLUGIN_AVX2", None)
            use_avx2 = host_config.has_avx2() if env_avx2 is None else env_avx2 == "True"
        if use_avx2 and not use_sse2:
            logger.error(
                "use_avx2=True disabled: incompatible with use_sse2=False")
            use_avx2 = False
        self.__use_avx2 = bool(use_avx2)

        env_avx512 = os.environ.get("HDF5PLUGIN_AVX512", None)
        use_avx512 = host_config.has_avx512() if env_avx512 is None else env_avx512 == "True"
        if use_avx512 and not (use_sse2 and use_avx2):
            logger.error(
                "use_avx512=True disabled: incompatible with use_sse2=False or use_avx2=False")
            use_avx512 = False
        self.__use_avx512 = bool(use_avx512)

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
        if self.__use_avx512:
            compile_args.extend(host_config.avx512_compile_args)
        if self.__use_native:
            compile_args.extend(host_config.native_compile_args)
        self.__compile_args = tuple(compile_args)

        self.embedded_filters = []

    hdf5_dir = property(lambda self: self.__hdf5_dir)
    filter_file_extension = property(lambda self: self.__filter_file_extension)
    use_cpp11 = property(lambda self: self.__use_cpp11)
    use_cpp14 = property(lambda self: self.__use_cpp14)
    use_sse2 = property(lambda self: self.__use_sse2)
    use_avx2 = property(lambda self: self.__use_avx2)
    use_avx512 = property(lambda self: self.__use_avx512)
    use_openmp = property(lambda self: self.__use_openmp)
    use_native = property(lambda self: self.__use_native)
    compile_args = property(lambda self: self.__compile_args)

    USE_BMI2 = bool(
            os.environ.get("HDF5PLUGIN_BMI2", 'True') == 'True'
            and sys.platform in ('linux', 'darwin')
        )
    """Whether to build with BMI2 instruction set or not (bool)"""

    INTEL_IPP_DIR = os.environ.get("HDF5PLUGIN_INTEL_IPP_DIR", None)
    """Root directory of Intel IPP or None to disable"""

    CONFIG_PY_TEMPLATE = """from collections import namedtuple

HDF5PluginBuildConfig = namedtuple('HDF5PluginBuildConfig', {field_names})
build_config = HDF5PluginBuildConfig(**{config})
"""

    def get_config_string(self):
        build_config = {
            'openmp': self.use_openmp,
            'native': self.use_native,
            'bmi2': self.USE_BMI2,
            'sse2': self.use_sse2,
            'avx2': self.use_avx2,
            'avx512': self.use_avx512,
            'cpp11': self.use_cpp11,
            'cpp14': self.use_cpp14,
            'ipp': self.INTEL_IPP_DIR is not None,
            'filter_file_extension': self.filter_file_extension,
            'embedded_filters': tuple(sorted(set(self.embedded_filters))),
        }
        return self.CONFIG_PY_TEMPLATE.format(
            field_names=tuple(build_config.keys()),
            config=str(build_config)
        )

    def has_config_changed(self) -> bool:
        """Returns whether config file needs to be changed or not."""
        try:
            with open(self.__config_file, 'r') as f:
                return f.read() != self.get_config_string()
        except:  # noqa
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
        ('hdf5=', None, "Deprecated, use HDF5PLUGIN_HDF5_DIR environment variable"),
        ('openmp=', None, "Deprecated, use HDF5PLUGIN_OPENMP environment variable"),
        ('native=', None, "Deprecated, use HDF5PLUGIN_NATIVE environment variable"),
        ('sse2=', None, "Deprecated, use HDF5PLUGIN_SSE2 environment variable"),
        ('avx2=', None, "Deprecated, use HDF5PLUGIN_AVX2 environment variable"),
        ('cpp11=', None, "Deprecated, use HDF5PLUGIN_CPP11 environment variable"),
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
        for argument in ('hdf5', 'cpp11', 'openmp', 'native', 'sse2', 'avx2'):
            if getattr(self, argument) is not None:
                logger.warning(
                    "--%s Deprecation warning: "
                    "use HDF5PLUGIN_%s environement variable.",
                    argument,
                    "HDF5_DIR" if argument == "hdf5" else argument.upper())
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

        if not self.hdf5plugin_config.use_cpp11:
            # Filter out C++11 libraries
            self.distribution.libraries = [
                (name, info) for name, info in self.distribution.libraries
                if '-std=c++11' not in info.get('cflags', [])]

            # Filter out C++11-only extensions
            self.distribution.ext_modules = [
                ext for ext in self.distribution.ext_modules
                if not (isinstance(ext, HDF5PluginExtension) and ext.cpp11_required)]

        if not self.hdf5plugin_config.use_cpp14:
            # Filter out C++14 libraries
            self.distribution.libraries = [
                (name, info) for name, info in self.distribution.libraries
                if '-std=c++14' not in info.get('cflags', [])
            ]

            # Filter out C++14-only extensions
            self.distribution.ext_modules = [
                ext for ext in self.distribution.ext_modules
                if '-std=c++14' not in ext.extra_compile_args
            ]

        self.hdf5plugin_config.embedded_filters = [
            ext.hdf5_plugin_name for ext in self.distribution.ext_modules
            if isinstance(ext, HDF5PluginExtension)
        ]

        logger.info("Build configuration: %s", self.hdf5plugin_config.get_config_string())

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
            config = self.distribution.get_command_obj("build").hdf5plugin_config

            cflags = list(build_info.get('cflags', []))

            # Add flags from build config
            cflags.extend(config.compile_args)

            if not config.use_openmp:  # Remove OpenMP flags
                cflags = [f for f in cflags if not f.endswith('openmp')]

            prefix = '/' if self.compiler.compiler_type == 'msvc' else '-'
            build_info['cflags'] = [
                f for f in cflags if f.startswith(prefix)
            ]

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

        if not self.depends:
            self.depends = list(itertools.chain.from_iterable(
                glob(f"{self.include_dirs}/*.h")))
            self.depends += list(itertools.chain.from_iterable(
                glob(f"{self.include_dirs}/*.hpp")))

        self.export_symbols.append('H5PLget_plugin_info')
        if not sys.platform == 'win32':
            self.export_symbols.append('init_filter')

        if sys.platform == 'win32':
            self.define_macros.append(('H5_BUILT_AS_DYNAMIC_LIB', None))
            self.libraries.append('hdf5')

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
            if sys.platform == 'win32':
                folder = 'windows'
            else:
                folder = 'darwin' if sys.platform == 'darwin' else 'linux'
            self.include_dirs.insert(0, os.path.join(hdf5_dir, 'include', folder))

        if sys.platform == 'win32':
            self.library_dirs.insert(0, os.path.join(hdf5_dir, 'lib'))
        self.include_dirs.insert(0, os.path.join(hdf5_dir, 'include'))

    @property
    def hdf5_plugin_name(self):
        """Return HDF5 plugin short name"""
        module_name = self.name.split('.')[-1]
        assert module_name.startswith('libh5')
        return module_name[5:]  # Strip libh5


def prefix(directory, files):
    """Add a directory as prefix to a list of files.

    :param str directory: Directory to add as prefix
    :param List[str] files: List of relative file path
    :rtype: List[str]
    """
    return ['/'.join((directory, f)) for f in files]


PLUGIN_LIB_DEPENDENCIES = dict()
"""Mapping plugin name to library name they depend on"""


# compression libs

def get_charls_clib(field=None):
    """CharLS static lib build config"""
    charls_dir = "src/charls/src"

    config = dict(
        sources=glob(f'{charls_dir}/*.cpp'),
        include_dirs=[charls_dir],
        cflags=['-std=c++11'],
    )

    if field is None:
        return 'charls', config
    return config[field]


# Define compilation arguments for Intel IPP
INTEL_IPP_INCLUDE_DIRS = []  # Intel IPP include directories
INTEL_IPP_EXTRA_LINK_ARGS = []  # Intel IPP extra link arguments
INTEL_IPP_LIBRARIES = []  # Intel IPP libraries to link
if BuildConfig.INTEL_IPP_DIR is not None:
    INTEL_IPP_INCLUDE_DIRS.append(f'{BuildConfig.INTEL_IPP_DIR}/include')
    arch = 'ia32' if HostConfig.ARCH == 'X86_32' else 'intel64'
    ipp_lib_dir = f'{BuildConfig.INTEL_IPP_DIR}/lib/{arch}'
    if not os.path.isdir(ipp_lib_dir):  # Happens on macos as only intel64 is available
        ipp_lib_dir = f'{BuildConfig.INTEL_IPP_DIR}/lib'
    INTEL_IPP_EXTRA_LINK_ARGS.append(f'-L{ipp_lib_dir}')
    INTEL_IPP_EXTRA_LINK_ARGS.append(f'/LIBPATH:{ipp_lib_dir}')
    INTEL_IPP_LIBRARIES.extend(['ippdc', 'ipps', 'ippvm', 'ippcore'])


def _get_lz4_ipp_clib(field=None):
    """LZ4 static lib using Intel IPP build config"""
    assert BuildConfig.INTEL_IPP_DIR is not None

    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    lz4_dir = 'src/lz4_ipp'

    config = dict(
        sources=glob(f'{lz4_dir}/*.c'),
        include_dirs=[lz4_dir] + INTEL_IPP_INCLUDE_DIRS,
        macros=[('WITH_IPP', 1)],
        cflags=cflags,
    )

    if field is None:
        return 'lz4', config
    if field == 'extra_link_args':
        return INTEL_IPP_EXTRA_LINK_ARGS
    if field == 'libraries':
        # Adding LZ4 here for it to be placed before IPP libs for gcc
        return ['lz4'] + INTEL_IPP_LIBRARIES
    return config[field]


def get_lz4_clib(field=None):
    """LZ4 static lib build config

    If HDFPLUGIN_IPP_DIR is set, it will use a patched LZ4 library to use Intel IPP.
    """
    if BuildConfig.INTEL_IPP_DIR is not None:
        return _get_lz4_ipp_clib(field)

    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    lz4_dir = glob('src/c-blosc2/internal-complibs/lz4*')[0]

    config = dict(
        sources=glob(f'{lz4_dir}/*.c'),
        include_dirs=[lz4_dir],
        cflags=cflags,
    )

    if field is None:
        return 'lz4', config
    if field == 'extra_link_args':
        return []
    if field == 'libraries':
        return []
    return config[field]


def get_snappy_clib(field=None):
    """snappy static lib build config"""
    snappy_dir = 'src/snappy'

    config = dict(
        sources=prefix(snappy_dir, [
            'snappy-c.cc',
            'snappy-sinksource.cc',
            'snappy-stubs-internal.cc',
            'snappy.cc',
        ]),
        include_dirs=[snappy_dir],
        cflags=['-std=c++11'],
    )

    if field is None:
        return 'snappy', config
    return config[field]


def get_zfp_clib(field=None):
    """ZFP static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    cflags += ['/Ox', '/fp:fast', '/openmp']

    zfp_dir = "src/zfp"

    config = dict(
        sources=glob(f'{zfp_dir}/src/*.c'),
        include_dirs=[f'{zfp_dir}/include'],
        macros=[('BIT_STREAM_WORD_TYPE', 'uint8')],
        cflags=cflags,
    )

    if field is None:
        return 'zfp', config
    return config[field]


def get_zlib_clib(field=None):
    """ZLib static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    zlib_dir = glob('src/c-blosc/internal-complibs/zlib*')[0]

    config = dict(
        sources=glob(f'{zlib_dir}/*.c'),
        include_dirs=[zlib_dir],
        cflags=cflags,
    )

    if field is None:
        return 'zlib', config
    return config[field]


def get_zstd_clib(field=None):
    """zstd static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    zstd_dir = glob('src/c-blosc2/internal-complibs/zstd*')[0]

    config = dict(
        sources=glob(f'{zstd_dir}/*/*.c'),
        include_dirs=[zstd_dir, f'{zstd_dir}/common'],
        macros=[] if BuildConfig.USE_BMI2 else [('ZSTD_DISABLE_ASM', 1)],
        cflags=cflags,
    )

    if field is None:
        return 'zstd', config
    if field == 'extra_objects':
        return glob(f'{zstd_dir}/*/*.S') if BuildConfig.USE_BMI2 else []
    return config[field]


# compression filter plugins


def get_blosc_plugin():
    """blosc plugin build config

    Plugin from https://github.com/Blosc/hdf5-blosc
    c-blosc from https://github.com/Blosc/c-blosc
    """
    blosc_dir = 'src/c-blosc/blosc'
    hdf5_blosc_dir = 'src/hdf5-blosc/src'

    # blosc sources
    sources = glob(f'{blosc_dir}/*.c')
    include_dirs = ['src/c-blosc', blosc_dir]
    define_macros = []
    extra_link_args = []
    libraries = []

    # compression libs
    # lz4
    include_dirs += get_lz4_clib('include_dirs')
    extra_link_args += get_lz4_clib('extra_link_args')
    libraries += get_lz4_clib('libraries')

    define_macros.append(('HAVE_LZ4', 1))

    # snappy
    cpp11_kwargs = {
        'include_dirs': get_snappy_clib('include_dirs'),
        'extra_link_args': ['-lstdc++'],
        'define_macros': [('HAVE_SNAPPY', 1)],
    }

    # zlib
    include_dirs += get_zlib_clib('include_dirs')
    define_macros.append(('HAVE_ZLIB', 1))

    # zstd
    include_dirs += get_zstd_clib('include_dirs')
    define_macros.append(('HAVE_ZSTD', 1))

    extra_compile_args = ['-std=gnu99']  # Needed to build manylinux1 wheels
    extra_compile_args += ['-O3', '-ffast-math']
    extra_compile_args += ['/Ox', '/fp:fast']
    extra_compile_args += ['-pthread']
    extra_link_args += ['-pthread']

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5blosc",
        sources=sources + prefix(
            hdf5_blosc_dir, ['blosc_filter.c', 'blosc_plugin.c']),
        extra_objects=get_zstd_clib('extra_objects'),
        include_dirs=include_dirs + [hdf5_blosc_dir],
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        libraries=libraries,
        sse2={'define_macros': [('SHUFFLE_SSE2_ENABLED', 1)]},
        avx2={'define_macros': [('SHUFFLE_AVX2_ENABLED', 1)]},
        cpp11=cpp11_kwargs,
    )


PLUGIN_LIB_DEPENDENCIES['blosc'] = 'snappy', 'lz4', 'zlib', 'zstd'


def get_blosc2_plugin():
    """blosc2 plugin build config

    Source from PyTables and c-blosc2
    """
    hdf5_blosc2_dir = 'src/PyTables/hdf5-blosc2/src'
    blosc2_dir = 'src/c-blosc2'

    # blosc sources
    sources = glob(f'{blosc2_dir}/blosc/*.c')
    include_dirs = [blosc2_dir, f'{blosc2_dir}/blosc', f'{blosc2_dir}/include']
    define_macros = [('SHUFFLE_NEON_ENABLED', 1)]
    extra_compile_args = []
    extra_link_args = []
    libraries = []

    if platform.machine() == 'ppc64le':
        define_macros.append(('SHUFFLE_ALTIVEC_ENABLED', 1))
        define_macros.append(('NO_WARN_X86_INTRINSICS', None))
    if HostConfig.ARCH == 'ARM_8':
        extra_compile_args += ['-flax-vector-conversions']
    if HostConfig.ARCH == 'ARM_7':
        extra_compile_args += ['-mfpu=neon', '-flax-vector-conversions']

    # compression libs
    # lz4
    include_dirs += get_lz4_clib('include_dirs')
    if BuildConfig.INTEL_IPP_DIR is None:
        extra_link_args += get_lz4_clib('extra_link_args')
        libraries += get_lz4_clib('libraries')
    else:
        include_dirs += INTEL_IPP_INCLUDE_DIRS
        extra_link_args += INTEL_IPP_EXTRA_LINK_ARGS
        libraries += INTEL_IPP_LIBRARIES
        define_macros.append(('HAVE_IPP', 1))

    # zlib
    include_dirs += get_zlib_clib('include_dirs')
    define_macros.append(('HAVE_ZLIB', 1))

    # zstd
    include_dirs += get_zstd_clib('include_dirs')
    define_macros.append(('HAVE_ZSTD', 1))

    extra_compile_args += ['-std=gnu99']  # Needed to build manylinux1 wheels
    extra_compile_args += ['-O3', '-ffast-math']
    extra_compile_args += ['/Ox', '/fp:fast']
    extra_compile_args += ['-pthread']
    extra_link_args += ['-pthread']

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5blosc2",
        sources=sources + \
            prefix(hdf5_blosc2_dir, ['blosc2_filter.c', 'blosc2_plugin.c']),
        extra_objects=get_zstd_clib('extra_objects'),
        include_dirs=include_dirs + [hdf5_blosc2_dir],
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        libraries=libraries,
        sse2={'define_macros': [('SHUFFLE_SSE2_ENABLED', 1)]},
        avx2={'define_macros': [('SHUFFLE_AVX2_ENABLED', 1)]},
    )


PLUGIN_LIB_DEPENDENCIES['blosc2'] = 'lz4', 'zlib', 'zstd'


def get_zstandard_plugin():
    """HDF5Plugin-Zstandard plugin build config"""
    zstandard_dir = 'src/HDF5Plugin-Zstandard'

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5zstd",
        sources=[f'{zstandard_dir}/zstd_h5plugin.c'],
        extra_objects=get_zstd_clib('extra_objects'),
        include_dirs=[zstandard_dir] + get_zstd_clib('include_dirs'),
    )


PLUGIN_LIB_DEPENDENCIES['zstd'] = ('zstd',)


def get_bitshuffle_plugin():
    """bitshuffle (+lz4 or zstd) plugin build config

    Plugins from https://github.com/kiyo-masui/bitshuffle
    """
    bithsuffle_dir = 'src/bitshuffle/src'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', '/openmp']

    define_macros=[("ZSTD_SUPPORT", 1)]
    if platform.machine() == 'ppc64le':
        define_macros.append(('NO_WARN_X86_INTRINSICS', None))  # P9 way to enable SSE2

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5bshuf",
        sources=prefix(bithsuffle_dir, [
            "bshuf_h5plugin.c",
            "bshuf_h5filter.c",
            "bitshuffle.c",
            "bitshuffle_core.c",
            "iochain.c",
        ]),
        extra_objects=get_zstd_clib('extra_objects'),
        include_dirs=[bithsuffle_dir] + get_lz4_clib('include_dirs') + get_zstd_clib('include_dirs'),
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args + get_lz4_clib('extra_link_args'),
        libraries=get_lz4_clib('libraries')
    )


PLUGIN_LIB_DEPENDENCIES['bshuf'] = ('lz4', 'zstd')


def get_lz4_plugin():
    """lz4 plugin build config

    Source from https://github.com/nexusformat/HDF5-External-Filter-Plugins
    """
    if sys.platform == 'darwin':
        extra_compile_args = ['-Wno-error=implicit-function-declaration']
    else:
        extra_compile_args = []

    libraries = ['Ws2_32'] if sys.platform == 'win32' else []
    libraries.extend(get_lz4_clib('libraries'))

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5lz4",
        sources=['src/LZ4/H5Zlz4.c', 'src/LZ4/lz4_h5plugin.c'],
        include_dirs=get_lz4_clib('include_dirs'),
        extra_compile_args=extra_compile_args,
        extra_link_args=get_lz4_clib('extra_link_args'),
        libraries=libraries,
    )


PLUGIN_LIB_DEPENDENCIES['lz4'] = ('lz4',)


def get_bzip2_plugin():
    """BZip2 plugin build config"""
    bzip2_dir = "src/bzip2"

    bzip2_extra_compile_args = [
        "-Wall",
        "-Winline",
        "-O2",
        "-g",
        "-D_FILE_OFFSET_BITS=64"
    ]

    sources = ['src/PyTables/src/H5Zbzip2.c', 'src/H5Zbzip2_plugin.c']
    sources += prefix(bzip2_dir, [
        "blocksort.c",
        "huffman.c",
        "crctable.c",
        "randtable.c",
        "compress.c",
        "decompress.c",
        "bzlib.c",
    ])

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5bzip2",
        sources=sources,
        include_dirs=['src/PyTables/src/', bzip2_dir],
        define_macros=[('HAVE_BZ2_LIB', 1)],
        extra_compile_args=bzip2_extra_compile_args,
    )


def get_fcidecomp_plugin():
    """FCIDECOMP plugin build config"""
    fcidecomp_dir = 'src/fcidecomp/FCIDECOMP_V1.0.2/Software/FCIDECOMP_SOURCES'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', '/openmp']

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5fcidecomp",
        sources=glob(f"{fcidecomp_dir}/fcicomp-*/src/*.c"),
        include_dirs=glob(f"{fcidecomp_dir}/fcicomp-*/include") + get_charls_clib('include_dirs'),
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        cpp11={'extra_link_args': ['-lstdc++']},
        cpp11_required=True,
        define_macros=[('CHARLS_STATIC', 1)],
    )


PLUGIN_LIB_DEPENDENCIES['fcidecomp'] = ('charls',)


def get_h5zfp_plugin():
    """H5Z-ZFP plugin build config"""
    h5zfp_dir = 'src/H5Z-ZFP/src'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', '/openmp']

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5zfp",
        sources=glob(f"{h5zfp_dir}/*.c"),
        include_dirs=[f"{h5zfp_dir}/src"] + get_zfp_clib('include_dirs'),
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    )


PLUGIN_LIB_DEPENDENCIES['zfp'] = ('zfp',)


def get_sz_plugin():
    """SZ library and its hdf5 filter plugin build config"""
    sz_dir = "src/SZ/sz"
    h5zsz_dir = "src/SZ/hdf5-filter/H5Z-SZ"

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', '/openmp', "-lm"]

    include_dirs = [f'{h5zsz_dir}/include']
    include_dirs += [sz_dir, f"{sz_dir}/include"]
    include_dirs += glob('src/SZ_extra/')
    include_dirs += get_zlib_clib('include_dirs')
    include_dirs += get_zstd_clib('include_dirs')

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5sz",
        sources=glob(f"{h5zsz_dir}/src/*.c") + glob(f"{sz_dir}/src/*.c"),
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        extra_objects=get_zstd_clib('extra_objects'),
    )


PLUGIN_LIB_DEPENDENCIES['sz'] = ('zlib', 'zstd')


def get_sz3_plugin():
    # SZ3 library and its hdf5 filter
    sz3_dir = "src/SZ3"
    h5z_sz3_dir = "src/SZ3/tools/H5Z-SZ3"

    include_dirs = [f"{sz3_dir}/include"]
    include_dirs += glob(f"{sz3_dir}/include/SZ3/*/")
    include_dirs += glob(f"{sz3_dir}/include/SZ3/utils/*/")
    # add version.hpp
    include_dirs.append("src/SZ3_extra")
    if sys.platform == 'darwin':
        # provide dummy omp.h
        include_dirs.append("src/SZ3_extra/darwin")
    include_dirs.append(f"{h5z_sz3_dir}/include")
    include_dirs += get_zstd_clib('include_dirs')

    extra_compile_args = ['-std=c++14', '-O3', '-ffast-math', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', '/openmp', "-lm"]

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5sz3",
        sources=[f"{h5z_sz3_dir}/src/H5Z_SZ3.cpp"],
        extra_objects=get_zstd_clib('extra_objects'),
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        cpp11_required=True,
    )


PLUGIN_LIB_DEPENDENCIES['sz3'] = ('zstd',)


def apply_filter_strip(libraries, extensions, dependencies):
    """Strip C libraries and extensions according to HDF5PLUGIN_STRIP env. var."""
    stripped_filters = set(
        name.strip().lower()
        for name in os.environ.get('HDF5PLUGIN_STRIP', '').split(',')
        if name.strip()
    )

    if 'all' in stripped_filters:
        return [], []

    # Filter out library that won't be used because of stripped filters
    lib_names = set(
        itertools.chain.from_iterable(
            lib_names for filter_name, lib_names in dependencies.items()
            if filter_name not in stripped_filters
        )
    )

    libraries = [
        lib for lib in libraries if lib[0] in lib_names
    ]

    # Filter out stripped filters
    extensions = [
        ext for ext in extensions
        if isinstance(ext, HDF5PluginExtension) and ext.hdf5_plugin_name not in stripped_filters
    ]
    return libraries, extensions


library_list = [
    get_charls_clib(),
    get_lz4_clib(),
    get_snappy_clib(),
    get_zfp_clib(),
    get_zlib_clib(),
    get_zstd_clib(),
]
libraries, extensions = apply_filter_strip(
    libraries=library_list,
    extensions=[
        get_bzip2_plugin(),
        get_lz4_plugin(),
        get_bitshuffle_plugin(),
        get_blosc_plugin(),
        get_blosc2_plugin(),
        get_fcidecomp_plugin(),
        get_h5zfp_plugin(),
        get_zstandard_plugin(),
        get_sz_plugin(),
        get_sz3_plugin(),
    ],
    dependencies=PLUGIN_LIB_DEPENDENCIES,
)


# hdf5 dynamic loading lib
def get_hdf5_dl_clib():
    include_dirs = []
    hdf5_dir = os.environ.get("HDF5PLUGIN_HDF5_DIR", None)
    if hdf5_dir is None:
        hdf5_dir = "src/hdf5"
        if sys.platform == 'win32':
            folder = 'windows'
        elif sys.platform == 'darwin':
            folder = 'darwin'
        else:
            folder = 'linux'
        include_dirs.append(f"{hdf5_dir}/include/{folder}")
    include_dirs.append(f"{hdf5_dir}/include")

    return ('hdf5_dl', {
        'sources': ['src/hdf5_dl.c'],
        'include_dirs': include_dirs,
        'macros': [('H5_USE_18_API', None)],
        'cflags': [],
    })


if extensions and not sys.platform == 'win32':
    libraries.append(get_hdf5_dl_clib())


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
url = 'https://github.com/silx-kit/hdf5plugin'
f = open("README.rst")
long_description = f.read()
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
               "Programming Language :: Python :: 3",
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
          python_requires='>=3.7',
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
