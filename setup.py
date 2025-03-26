# coding: utf-8
# /*##########################################################################
#
# Copyright (c) 2016-2025 European Synchrotron Radiation Facility
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
from __future__ import annotations

__authors__ = ["V.A. Sole", "T. Vincent"]
__license__ = "MIT"
__date__ = "05/12/2022"


from functools import lru_cache
from glob import glob
from pathlib import Path
import itertools
import logging
import os
import sys
import sysconfig
import tempfile
import platform
from functools import cached_property
from setuptools import setup, Distribution, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py
from setuptools.command.build_clib import build_clib
from setuptools.command.build import build
from setuptools.errors import CompileError
from wheel.bdist_wheel import bdist_wheel

try:
    import pkgconfig
except ImportError:
    pkgconfig = None

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    import cpuinfo
except ModuleNotFoundError:
    raise ModuleNotFoundError("py-cpuinfo is required to build hdf5plugin, please install it.")
except Exception:  # cpuinfo raises Exception for unsupported architectures
    logger.warning("Architecture is not supported by cpuinfo")
    cpuinfo = None
    cpuinfo_parse_arch = None
else:
    from cpuinfo.cpuinfo import _parse_arch as cpuinfo_parse_arch


class BDistWheel(bdist_wheel):
    """Override bdist_wheel to handle as pure python package"""

    def get_tag(self):
        """Override the python and abi tag generation"""
        return self.python_tag, "none", super().get_tag()[-1]


# Probe host capabilities and manage build config

def get_compiler(compiler):
    """Returns an initialized compiler

    Taken from https://github.com/pypa/setuptools/issues/2806#issuecomment-961805789
    """
    build_ext = Distribution().get_command_obj("build_ext")
    build_ext.compiler = compiler
    build_ext.finalize_options()
    # register an extension to ensure a compiler is created
    build_ext.extensions = [Extension("ignored", ["ignored.c"])]
    # disable building fake extensions
    build_ext.build_extensions = lambda: None
    # run to populate self.compiler
    build_ext.run()
    return build_ext.compiler


def check_compile_flags(compiler, *flags, extension='.c', source=None):
    """Try to compile an empty file to check for compiler args

    :param distutils.ccompiler.CCompiler compiler: The compiler to use
    :param flags: Flags argument to pass to compiler
    :param str extension: Source file extension (default: '.c')
    :param source: Source code to compile (default: dummy C function)
    :returns: Whether or not compilation was successful
    :rtype: bool
    """
    if source is None:
        source = 'int main (int argc, char **argv) { return 0; }\n'
    with tempfile.TemporaryDirectory() as tmp_dir:
        # Create empty source file
        tmp_file = Path(tmp_dir) / f"source{extension}"
        tmp_file.write_text(source)
        try:
            compiler.compile([str(tmp_file)], output_dir=tmp_dir, extra_postargs=list(flags))
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
        self.__compiler = get_compiler(compiler)

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
            self.ssse3_compile_args = ('-mssse3',)  # There is no /arch:SSSE3
        else:
            self.ssse3_compile_args = ()

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

    @lru_cache()
    def get_cpp20_flag(self) -> str | None:
        """Returns C++20 compiler flag or None if not supported"""
        if self.__compiler.compiler_type == 'msvc':
            # C++20 available since Visual Studio C++ 2019 v16.11
            # Lack of support of the compilation flag do not raise an error
            # So instead check the _MSVC_LANG macro
            # See: https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170#c-standards-support
            is_available = check_compile_flags(
                self.__compiler,
                '/std:c++20',
                extension='.cc',
                source="""
                    #if _MSVC_LANG != 202002L
                    #error C++20 is not supported
                    #endif
                    int main (int argc, char **argv) { return 0; }
                """,
            )
            return "/std:c++20" if is_available else None

        # -std=c++20 if clang>=10 or gcc>=10 else -std=c++2a
        for flag in ('-std=c++20', '-std=c++2a'):
            if check_compile_flags(self.__compiler, flag, extension='.cc'):
                break
        else:  # Check failed for both flags
            return None

        if sys.platform != 'darwin':
            return flag

        # Check macos min version >= 10.13:
        # 10.13 does not fully support C++20, but is enough to build the SPERR library
        is_available = check_compile_flags(
            self.__compiler,
            flag,
            extension='.cc',
            source="""
                #include "AvailabilityMacros.h"
                #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_13
                #error C++20 is not supported: macOS 10.13 at least is required
                #endif
                int main (int argc, char **argv) { return 0; }
            """,
        )
        return flag if is_available else None

    def has_cpp20(self) -> bool:
        """Check C++20 availability on host"""
        return self.get_cpp20_flag() is not None

    def _has_x86_simd(self, *flags) -> bool:
        """Check x86 SIMD availability on host"""
        if self.ARCH not in ('X86_32', 'X86_64'):
            return False
        if not all(has_cpu_flag(flag) for flag in flags):
            return False
        if self.__compiler.compiler_type == "msvc":
            return True
        return check_compile_flags(self.__compiler, *(f"-m{flag}" for flag in flags))

    def has_sse2(self) -> bool:
        """Check SSE2 availability on host"""
        return self._has_x86_simd('sse2')

    def has_ssse3(self) -> bool:
        """Check SSSE3 availability on host"""
        return self._has_x86_simd('ssse3')

    def has_avx2(self) -> bool:
        """Check AVX2 availability on host"""
        return self._has_x86_simd('avx2')

    def has_avx512(self) -> bool:
        """Check AVX512 "F" and "BW" instruction sets availability on host"""
        return self._has_x86_simd('avx512f', 'avx512bw')

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
    def __init__(self, config_file: Path, compiler=None):
        self.__config_file = config_file
        self.__host_config = HostConfig(compiler)
        self.embedded_filters = []

    @staticmethod
    def get_hdf5_library_dirs() -> list[str]:
        """Returns list of HDF5 library directories"""
        if sys.platform != 'win32':
            return []
        hdf5_dir = os.environ.get("HDF5PLUGIN_HDF5_DIR", "src/hdf5")
        return [f"{hdf5_dir}/lib"]

    @staticmethod
    def get_hdf5_include_dirs() -> list[str]:
        """Returns list of HDF5 include directories"""
        try:
            hdf5_dir = os.environ["HDF5PLUGIN_HDF5_DIR"]
        except KeyError:
            hdf5_dir = "src/hdf5"
        else:  # HDF5 directory define by environment variable
            return [f"{hdf5_dir}/include"]

        # Add folder containing H5pubconf.h
        if sys.platform == 'win32':
            folder = 'windows'
        elif sys.platform == 'darwin':
            folder = 'darwin'
        else:
            folder = 'linux'
        return [f"{hdf5_dir}/include", f"{hdf5_dir}/include/{folder}"]

    @property
    def filter_file_extension(self) -> str:
        """File extension to use for filter shared libraries"""
        return self.__host_config.get_shared_lib_extension()

    @property
    def compile_args(self) -> tuple[str, ...]:
        """Returns compile args to use"""
        compile_args = []
        if self.use_sse2:
            compile_args.extend(self.__host_config.sse2_compile_args)
        if self.use_ssse3:
            compile_args.extend(self.__host_config.ssse3_compile_args)
        if self.use_avx2:
            compile_args.extend(self.__host_config.avx2_compile_args)
        if self.use_avx512:
            compile_args.extend(self.__host_config.avx512_compile_args)
        if self.use_native:
            compile_args.extend(self.__host_config.native_compile_args)
        return tuple(compile_args)

    @property
    def cpp20_compile_arg(self) -> str | None:
        """Returns C++20 compilation flag or None if not supported"""
        return self.__host_config.get_cpp20_flag()

    @lru_cache(maxsize=None)
    def _is_enabled(self, feature: str) -> bool:
        """Returns True if given compilation feature is enabled.

        It first checks if the corresponding HDF5PLUGIN_* environment variable is set.
        If it is not set, it checks if both the compiler and the host support the feature.
        """
        assert feature in ("cpp11", "cpp14", "cpp20", "sse2", "ssse3", "avx2", "avx512", "openmp", "native")
        try:
            return os.environ[f"HDF5PLUGIN_{feature.upper()}"] == "True"
        except KeyError:
            pass
        check = getattr(self.__host_config, f"has_{feature.lower()}")
        return check()

    use_cpp11 = property(lambda self: self._is_enabled('cpp11'))
    use_cpp14 = property(lambda self: self._is_enabled('cpp14'))
    use_cpp20 = property(lambda self: self._is_enabled('cpp20'))
    use_sse2 = property(lambda self: self._is_enabled('sse2'))
    use_openmp = property(lambda self: self._is_enabled('openmp'))
    use_native = property(lambda self: self._is_enabled('native'))

    @cached_property
    def use_ssse3(self) -> bool:
        enabled = self._is_enabled('ssse3')
        if enabled and not self.use_sse2:
            logger.error("use_ssse3=True disabled: incompatible with use_sse2=False")
            return False
        return enabled

    @cached_property
    def use_avx2(self) -> bool:
        enabled = self._is_enabled('avx2')
        if enabled and not (self.use_sse2 and self.use_ssse3):
            logger.error(
                "use_avx2=True disabled: incompatible with use_sse2=False and use_ssse3=False")
            return False
        return enabled

    @cached_property
    def use_avx512(self) -> bool:
        enabled = self._is_enabled('avx512')
        if enabled and not (self.use_sse2 and self.use_ssse3 and self.use_avx2):
            logger.error(
                "use_avx512=True disabled: incompatible with use_sse2=False, use_ssse3=False and use_avx2=False")
            return False
        return enabled

    USE_BMI2 = bool(
        os.environ.get("HDF5PLUGIN_BMI2", 'True') == 'True'
        and sys.platform in ('linux', 'darwin')
    )
    """Whether to build with BMI2 instruction set or not (bool)"""

    INTEL_IPP_DIR = os.environ.get("HDF5PLUGIN_INTEL_IPP_DIR", None)
    """Root directory of Intel IPP or None to disable"""

    def get_config_string(self) -> str:
        build_config = {
            'openmp': self.use_openmp,
            'native': self.use_native,
            'bmi2': self.USE_BMI2,
            'sse2': self.use_sse2,
            'ssse3': self.use_ssse3,
            'avx2': self.use_avx2,
            'avx512': self.use_avx512,
            'cpp11': self.use_cpp11,
            'cpp14': self.use_cpp14,
            'cpp20': self.use_cpp20,
            'ipp': self.INTEL_IPP_DIR is not None,
            'filter_file_extension': self.filter_file_extension,
            'embedded_filters': tuple(sorted(set(self.embedded_filters))),
        }
        return f"""from collections import namedtuple

HDF5PluginBuildConfig = namedtuple('HDF5PluginBuildConfig', {tuple(build_config.keys())})
build_config = HDF5PluginBuildConfig(**{build_config})
"""

    def has_config_changed(self) -> bool:
        """Returns whether config file needs to be changed or not."""
        try:
            return self.__config_file.read_text() != self.get_config_string()
        except:  # noqa
            pass
        return True

    def save_config(self) -> None:
        """Save config as a dict in a python file"""
        self.__config_file.write_text(self.get_config_string())


# Plugins

class Build(build):
    """Build command with extra options used by PluginBuildExt"""

    def finalize_options(self):
        build.finalize_options(self)
        self.hdf5plugin_config = BuildConfig(
            config_file=Path(self.build_lib) / "hdf5plugin" / "_config.py",
            compiler=self.compiler,
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
                and not (isinstance(ext, HDF5PluginExtension) and ext.cpp14_required)
            ]

        if not self.hdf5plugin_config.use_cpp20:
            cpp20_flags = set(["/std:c++20", "-std=c++20"])

            # Filter out C++20 libraries
            self.distribution.libraries = [
                (name, info) for name, info in self.distribution.libraries
                if cpp20_flags.isdisjoint(info.get('cflags', []))
            ]

            # Filter out C++20-only extensions
            self.distribution.ext_modules = [
                ext for ext in self.distribution.ext_modules
                if not (isinstance(ext, HDF5PluginExtension) and ext.cpp20_required)]

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
            cflags = [
                f for f in cflags if f.startswith(prefix)
            ]

            # Patch C++20 flag for older gcc/clang support
            if '-std=c++20' in cflags:
                if config.cpp20_compile_arg is None:
                    raise RuntimeError("Cannot compile C++20 code with current compiler")
                cflags = [f if f != '-std=c++20' else config.cpp20_compile_arg for f in cflags]

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

    def __init__(
            self,
            name,
            sse2=None,
            avx2=None,
            cpp11=None,
            cpp11_required=False,
            cpp14_required=False,
            cpp20_required=False,
            **kwargs
    ):
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

        # Add HDF5 library directories
        self.include_dirs = BuildConfig.get_hdf5_include_dirs() + self.include_dirs
        self.library_dirs = BuildConfig.get_hdf5_library_dirs() + self.library_dirs

        self.sse2 = sse2 if sse2 is not None else {}
        self.avx2 = avx2 if avx2 is not None else {}
        self.cpp11 = cpp11 if cpp11 is not None else {}
        self.cpp11_required = cpp11_required
        self.cpp14_required = cpp14_required
        self.cpp20_required = cpp20_required

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


def _get_bz2_clib(field=None):
    """BZip2 static lib build config"""
    bzip2_dir = "src/bzip2"

    config = dict(
        sources=prefix(bzip2_dir, [
            "blocksort.c",
            "huffman.c",
            "crctable.c",
            "randtable.c",
            "compress.c",
            "decompress.c",
            "bzlib.c",
        ]),
        include_dirs=[bzip2_dir],
        macros=[],
        cflags=[
            "-Wall",
            "-Winline",
            "-O2",
            "-g",
            "-D_FILE_OFFSET_BITS=64"
        ],
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


def _get_charls_clib(field=None):
    """CharLS static lib build config"""
    charls_dir = "src/charls/src"

    config = dict(
        sources=glob(f'{charls_dir}/*.cpp'),
        include_dirs=["src/charls/include", "src/charls/include/charls", charls_dir],
        macros=[("CHARLS_STATIC", 1), ("CHARLS_LIBRARY_BUILD", 1)],
        cflags=['-std=c++14'],
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
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
        return config
    if field == 'extra_link_args':
        return INTEL_IPP_EXTRA_LINK_ARGS
    if field == 'libraries':
        # Adding LZ4 here for it to be placed before IPP libs for gcc
        return ['lz4'] + INTEL_IPP_LIBRARIES
    return config[field]


def _get_lz4_clib(field=None):
    """LZ4 static lib build config

    If HDFPLUGIN_IPP_DIR is set, it will use a patched LZ4 library to use Intel IPP.
    """
    if BuildConfig.INTEL_IPP_DIR is not None:
        return _get_lz4_ipp_clib(field)

    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    folders = glob('src/c-blosc2/internal-complibs/lz4*')
    lz4_dir = folders[0] if folders else "src/no-folder"

    config = dict(
        sources=glob(f'{lz4_dir}/*.c'),
        include_dirs=[lz4_dir],
        macros=[],
        cflags=cflags,
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


def _get_snappy_clib(field=None):
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
        macros=[],
        cflags=['-std=c++11'],
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


def _get_sperr_clib(field=None):
    """sperr static lib and sperr filter C++20 source file build config"""
    sperr_dir = "src/SPERR"

    config = dict(
        sources=glob(f"{sperr_dir}/src/*.cpp"),
        include_dirs=[f"{sperr_dir}/include"],
        macros=[
            ("SPERR_VERSION_MAJOR", 0),  # Check project(SPERR VERSION ... in src/SPERR/CMakeLists.txt
            ("USE_VANILLA_CONFIG", 1),
        ],
        cflags=["-std=c++20", "/std:c++20"],
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


def _get_sperr_filter_clib(field=None):
    """sperr filter C++20 source file build config"""
    h5z_sperr_dir = "src/H5Z-SPERR"

    config = dict(
        sources=[f"{h5z_sperr_dir}/src/h5zsperr_helper.cpp"],
        include_dirs=BuildConfig.get_hdf5_include_dirs() + ["src/SPERR/include", f"{h5z_sperr_dir}/include"],
        macros=[],
        cflags=["-std=c++20", "/std:c++20"],
    )

    if sys.platform == 'win32':
        # h5zsperr_helper.cpp is part of the plugin
        config["macros"].append(('H5_BUILT_AS_DYNAMIC_LIB', None))

    if field is None:
        return config
    raise RuntimeError("SPERR filter C++ source lib config is not expected to be used this way")


def _get_zfp_clib(field=None):
    """ZFP static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    # Use /Ob1 to fix ZFP test failing with Visual Studio 2022: MSVC 14.40.33807
    # See https://github.com/LLNL/zfp/issues/222
    cflags += ['/Ob1', '/fp:fast', '/openmp']

    zfp_dir = "src/zfp"

    config = dict(
        sources=glob(f'{zfp_dir}/src/*.c'),
        include_dirs=[f'{zfp_dir}/include'],
        macros=[('BIT_STREAM_WORD_TYPE', 'uint8')],
        cflags=cflags,
    )

    if field is None:
        return config
    if field == 'extra_link_args':
        return ['-fopenmp']
    if field == 'libraries':
        return []
    return config[field]


def _get_zlib_clib(field=None):
    """ZLib static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    folders = glob('src/c-blosc/internal-complibs/zlib*')
    zlib_dir = folders[0] if folders else "src/no-folder"

    config = dict(
        sources=glob(f'{zlib_dir}/*.c'),
        include_dirs=[zlib_dir],
        macros=[],
        cflags=cflags,
    )

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


def _get_zstd_clib(field=None):
    """zstd static lib build config"""
    cflags = ['-O3', '-ffast-math', '-std=gnu99']
    cflags += ['/Ox', '/fp:fast']

    folders = glob('src/c-blosc2/internal-complibs/zstd*')
    zstd_dir = folders[0] if folders else "src/no-folder"

    config = dict(
        sources=glob(f'{zstd_dir}/*/*.c'),
        include_dirs=[zstd_dir, f'{zstd_dir}/common'],
        macros=[] if BuildConfig.USE_BMI2 else [('ZSTD_DISABLE_ASM', 1)],
        cflags=cflags,
    )

    if field == 'extra_objects':
        return glob(f'{zstd_dir}/*/*.S') if BuildConfig.USE_BMI2 else []

    if field is None:
        return config
    if field in ('extra_link_args', 'libraries'):
        return []
    return config[field]


_EMBEDDED_CLIB_CONFIG_GETTERS = {
    "bz2": _get_bz2_clib,
    "charls": _get_charls_clib,
    "lz4": _get_lz4_clib,
    "snappy": _get_snappy_clib,
    "sperr": _get_sperr_clib,
    "sperr_filter": _get_sperr_filter_clib,
    "zfp": _get_zfp_clib,
    "zlib": _get_zlib_clib,
    "zstd": _get_zstd_clib,
}
_CLIB_NAMES = {"blosc", "blosc2"} | set(_EMBEDDED_CLIB_CONFIG_GETTERS.keys()) - {"sperr_filter"}


@lru_cache()
def get_system_clib_names():
    """Returns the set of clib names that should not be embedded"""
    system_clibs = set(
        name.strip().lower()
        for name in os.environ.get('HDF5PLUGIN_SYSTEM_LIBRARIES', '').split(',')
        if name.strip()
    )
    if not system_clibs <= _CLIB_NAMES:
        raise RuntimeError(f"Unexpected names in HDF5PLUGIN_SYSTEM_LIBRARIES: {system_clibs - _CLIB_NAMES}")
    return system_clibs


@lru_cache()
def _get_clib_config_from_pkgconfig(libname):
    if pkgconfig is None:
        raise RuntimeError("pkgconfig is required to build against system libraries")
    if not pkgconfig.exists(libname):
        if pkgconfig.exists(f"lib{libname}"):
            libname = f"lib{libname}"
        else:
            raise RuntimeError(f"pkgconfig: (lib){libname} not found")

    include_dir = pkgconfig.variables(libname).get('includedir', None)

    return {
        'include_dirs': [include_dir] if include_dir is not None else [],
        'extra_link_args': pkgconfig.libs(libname).split(" "),
        'libraries': [],
        'macros': [],
        'extra_objects': [],
    }


def get_clib_config(libname, field=None):
    """Returns the configuration or one of it's field for the given clib"""
    if libname in get_system_clib_names():
        return _get_clib_config_from_pkgconfig(libname)[field]

    get_embedded_config = _EMBEDDED_CLIB_CONFIG_GETTERS[libname]
    return get_embedded_config(field)


# compression filter plugins


def _get_blosc_plugin():
    """blosc plugin build config

    Plugin from https://github.com/Blosc/hdf5-blosc
    c-blosc from https://github.com/Blosc/c-blosc
    """
    hdf5_blosc_dir = 'src/hdf5-blosc/src'
    hdf5_blosc_sources = prefix(hdf5_blosc_dir, ['blosc_filter.c', 'blosc_plugin.c'])

    extra_compile_args = ['-std=gnu99']  # Needed to build manylinux1 wheels
    extra_compile_args += ['-O3', '-ffast-math']
    extra_compile_args += ['/Ox', '/fp:fast']
    extra_compile_args += ['-pthread']
    extra_link_args = ['-pthread']

    if "blosc" in get_system_clib_names():
        return HDF5PluginExtension(
            "hdf5plugin.plugins.libh5blosc",
            sources=hdf5_blosc_sources,
            include_dirs=get_clib_config("blosc", "include_dirs") + [hdf5_blosc_dir],
            extra_compile_args=extra_compile_args,
            extra_link_args=get_clib_config("blosc", "extra_link_args") + extra_link_args,
            libraries=get_clib_config("blosc", "libraries"),
        )

    blosc_dir = 'src/c-blosc/blosc'

    # blosc sources
    sources = glob(f'{blosc_dir}/*.c')
    include_dirs = ['src/c-blosc', blosc_dir]
    define_macros = []
    libraries = []

    # compression libs
    # lz4
    include_dirs += get_clib_config('lz4', 'include_dirs')
    extra_link_args += get_clib_config('lz4', 'extra_link_args')
    libraries += get_clib_config('lz4', 'libraries')
    define_macros.append(('HAVE_LZ4', 1))
    # snappy
    cpp11_kwargs = {
        'include_dirs': get_clib_config('snappy', 'include_dirs'),
        'extra_link_args': ['-lstdc++'],
        'define_macros': [('HAVE_SNAPPY', 1)],
    }
    # zlib
    include_dirs += get_clib_config('zlib', 'include_dirs')
    extra_link_args += get_clib_config('zlib', 'extra_link_args')
    libraries += get_clib_config('zlib', 'libraries')
    define_macros.append(('HAVE_ZLIB', 1))
    # zstd
    include_dirs += get_clib_config('zstd', 'include_dirs')
    extra_link_args += get_clib_config('zstd', 'extra_link_args')
    libraries += get_clib_config('zstd', 'libraries')
    define_macros.append(('HAVE_ZSTD', 1))

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5blosc",
        sources=sources + hdf5_blosc_sources,
        extra_objects=get_clib_config('zstd', 'extra_objects'),
        include_dirs=include_dirs + [hdf5_blosc_dir],
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        libraries=libraries,
        sse2={'define_macros': [('SHUFFLE_SSE2_ENABLED', 1)]},
        avx2={'define_macros': [('SHUFFLE_AVX2_ENABLED', 1)]},
        cpp11=cpp11_kwargs,
)


if 'blosc' in get_system_clib_names():
    PLUGIN_LIB_DEPENDENCIES['blosc'] = ()
else:
    PLUGIN_LIB_DEPENDENCIES['blosc'] = 'snappy', 'lz4', 'zlib', 'zstd'


def _get_blosc2_plugin():
    """blosc2 plugin build config

    Source from PyTables and c-blosc2
    """
    hdf5_blosc2_dir = 'src/PyTables/hdf5-blosc2/src'
    hdf5_blosc2_sources = prefix(hdf5_blosc2_dir, ['blosc2_filter.c', 'blosc2_plugin.c'])

    extra_compile_args = ['-O3', '-std=gnu99']
    extra_compile_args += ['/Ox']
    extra_compile_args += ['-pthread']
    extra_link_args = ['-pthread']

    if "blosc2" in get_system_clib_names():
        return HDF5PluginExtension(
            "hdf5plugin.plugins.libh5blosc2",
            sources=hdf5_blosc2_sources,
            include_dirs=get_clib_config("blosc2", "include_dirs") + [hdf5_blosc2_dir],
            extra_compile_args=extra_compile_args,
            extra_link_args=get_clib_config("blosc2", "extra_link_args") + extra_link_args,
            libraries=get_clib_config("blosc2", "libraries"),
        )

    # blosc sources
    blosc2_dir = 'src/c-blosc2'
    plugins_dir = f'{blosc2_dir}/plugins'

    sources = glob(f'{blosc2_dir}/blosc/*.c')
    sources += [  # Add embedded codecs, filters and tuners
        src_file
        for src_file in glob(f'{plugins_dir}/*.c') + glob(f'{plugins_dir}/*/*.c') + glob(f'{plugins_dir}/*/*/*.c')
        if not os.path.basename(src_file).startswith("test")
    ]
    sources += glob(f'{plugins_dir}/codecs/zfp/src/*.c')  # Add ZFP embedded sources
    include_dirs = [
        blosc2_dir,
        f'{blosc2_dir}/blosc',
        f'{blosc2_dir}/include',
        f'{blosc2_dir}/plugins/codecs/zfp/include',
    ]
    define_macros = [('HAVE_PLUGINS', 1)]
    if platform.machine() == 'ppc64le':
        define_macros.append(('SHUFFLE_ALTIVEC_ENABLED', 1))
        define_macros.append(('NO_WARN_X86_INTRINSICS', None))
    else:
        define_macros.append(('SHUFFLE_SSE2_ENABLED', 1))
        define_macros.append(('SHUFFLE_AVX2_ENABLED', 1))
        define_macros.append(('SHUFFLE_AVX512_ENABLED', 1))
        define_macros.append(('SHUFFLE_NEON_ENABLED', 1))

    libraries = []

    if HostConfig.ARCH == 'ARM_8':
        extra_compile_args += ['-flax-vector-conversions']
    if HostConfig.ARCH == 'ARM_7':
        extra_compile_args += ['-mfpu=neon', '-flax-vector-conversions']
    # compression libs
    # lz4
    include_dirs += get_clib_config('lz4', 'include_dirs')
    if BuildConfig.INTEL_IPP_DIR is None:
        extra_link_args += get_clib_config('lz4', 'extra_link_args')
        libraries += get_clib_config('lz4', 'libraries')
    else:
        include_dirs += INTEL_IPP_INCLUDE_DIRS
        extra_link_args += INTEL_IPP_EXTRA_LINK_ARGS
        libraries += INTEL_IPP_LIBRARIES
        define_macros.append(('HAVE_IPP', 1))
    # zlib
    include_dirs += get_clib_config('zlib', 'include_dirs')
    extra_link_args += get_clib_config('zlib', 'extra_link_args')
    libraries += get_clib_config('zlib', 'libraries')
    define_macros.append(('HAVE_ZLIB', 1))
    # zstd
    include_dirs += get_clib_config('zstd', 'include_dirs')
    extra_link_args += get_clib_config('zstd', 'extra_link_args')
    libraries += get_clib_config('zstd', 'libraries')
    define_macros.append(('HAVE_ZSTD', 1))

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5blosc2",
        sources=sources + hdf5_blosc2_sources,
        extra_objects=get_clib_config('zstd', 'extra_objects'),
        include_dirs=include_dirs + [hdf5_blosc2_dir],
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        libraries=libraries,
        sse2={'define_macros': [('SHUFFLE_SSE2_ENABLED', 1)]},
        avx2={'define_macros': [('SHUFFLE_AVX2_ENABLED', 1)]},
    )


if 'blosc2' in get_system_clib_names():
    PLUGIN_LIB_DEPENDENCIES['blosc2'] = ()
else:
    PLUGIN_LIB_DEPENDENCIES['blosc2'] = 'lz4', 'zlib', 'zstd'


def _get_zstandard_plugin():
    """HDF5Plugin-Zstandard plugin build config"""
    zstandard_dir = 'src/HDF5Plugin-Zstandard'

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5zstd",
        sources=[f'{zstandard_dir}/zstd_h5plugin.c'],
        extra_objects=get_clib_config('zstd', 'extra_objects'),
        include_dirs=[zstandard_dir] + get_clib_config('zstd', 'include_dirs'),
        extra_link_args=get_clib_config('zstd', 'extra_link_args'),
        libraries=get_clib_config('zstd', 'libraries'),
    )


PLUGIN_LIB_DEPENDENCIES['zstd'] = ('zstd',)


def _get_bitshuffle_plugin():
    """bitshuffle (+lz4 or zstd) plugin build config

    Plugins from https://github.com/kiyo-masui/bitshuffle
    """
    bithsuffle_dir = 'src/bitshuffle/src'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp']

    define_macros = [("ZSTD_SUPPORT", 1)]
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
        extra_objects=get_clib_config('zstd', 'extra_objects'),
        include_dirs=[bithsuffle_dir] + get_clib_config('lz4', 'include_dirs') + get_clib_config('zstd', 'include_dirs'),
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args + get_clib_config('lz4', 'extra_link_args') + get_clib_config('zstd', 'extra_link_args'),
        libraries=get_clib_config('lz4', 'libraries') + get_clib_config('zstd', 'libraries'),
    )


PLUGIN_LIB_DEPENDENCIES['bshuf'] = ('lz4', 'zstd')


def _get_lz4_plugin():
    """lz4 plugin build config

    Source from https://github.com/nexusformat/HDF5-External-Filter-Plugins
    """
    if sys.platform == 'darwin':
        extra_compile_args = ['-Wno-error=implicit-function-declaration']
    else:
        extra_compile_args = []

    libraries = ['Ws2_32'] if sys.platform == 'win32' else []
    libraries.extend(get_clib_config('lz4', 'libraries'))

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5lz4",
        sources=['src/LZ4/H5Zlz4.c', 'src/LZ4/lz4_h5plugin.c'],
        include_dirs=get_clib_config('lz4', 'include_dirs'),
        extra_compile_args=extra_compile_args,
        extra_link_args=get_clib_config('lz4', 'extra_link_args'),
        libraries=libraries,
    )


PLUGIN_LIB_DEPENDENCIES['lz4'] = ('lz4',)


def _get_bzip2_plugin():
    """BZip2 plugin build config"""
    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5bzip2",
        sources=['src/PyTables/src/H5Zbzip2.c', 'src/H5Zbzip2_plugin.c'],
        include_dirs=['src/PyTables/src/'] + get_clib_config('bz2', 'include_dirs'),
        define_macros=[('HAVE_BZ2_LIB', 1)],
        extra_compile_args=[],
        extra_link_args=get_clib_config('bz2', 'extra_link_args'),
        libraries=get_clib_config('bz2', 'libraries'),
    )


PLUGIN_LIB_DEPENDENCIES['bzip2'] = ('bz2',)


def _get_fcidecomp_plugin():
    """FCIDECOMP plugin build config"""
    fcidecomp_dir = 'src/fcidecomp/src/fcidecomp'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-lstdc++', '-fopenmp']

    macros = [('LOGGING', 1)]
    if 'charls' not in get_system_clib_names():
        macros.append(('CHARLS_STATIC', 1))

    charls_include_dirs = get_clib_config('charls', 'include_dirs')
    # fcidecomp includes "charls.h" instead of "charls/charls.h"
    charls_include_dirs += [os.path.join(d, "charls") for d in get_clib_config('charls', 'include_dirs')]

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5fcidecomp",
        sources=glob(f"{fcidecomp_dir}/fcicomp-*/src/*.c"),
        include_dirs=glob(f"{fcidecomp_dir}/fcicomp-*/include") + charls_include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args + get_clib_config('charls', 'extra_link_args'),
        libraries=get_clib_config('charls', 'libraries'),
        cpp14_required=True,
        define_macros=macros,
    )


PLUGIN_LIB_DEPENDENCIES['fcidecomp'] = ('charls',)


def _get_h5zfp_plugin():
    """H5Z-ZFP plugin build config"""
    h5zfp_dir = 'src/H5Z-ZFP/src'

    extra_compile_args = ['-O3', '-ffast-math', '-std=c99']
    extra_compile_args += ['/Ox', '/fp:fast']

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5zfp",
        sources=glob(f"{h5zfp_dir}/*.c"),
        include_dirs=[f"{h5zfp_dir}/src"] + get_clib_config('zfp', 'include_dirs'),
        extra_compile_args=extra_compile_args,
        extra_link_args=get_clib_config('zfp', 'extra_link_args'),
        libraries=get_clib_config('zfp', 'libraries'),
    )


PLUGIN_LIB_DEPENDENCIES['zfp'] = ('zfp',)


def _get_sz_plugin():
    """SZ library and its hdf5 filter plugin build config"""
    sz_dir = "src/SZ/sz"
    h5zsz_dir = "src/SZ/hdf5-filter/H5Z-SZ"

    extra_compile_args = ['-O3', '-std=c99', '-fopenmp']
    extra_compile_args += ['/Ox', '/openmp']

    extra_link_args = ['-fopenmp', "-lm"]
    extra_link_args += get_clib_config('zlib', 'extra_link_args')
    extra_link_args += get_clib_config('zstd', 'extra_link_args')

    include_dirs = [f'{h5zsz_dir}/include']
    include_dirs += [sz_dir, f"{sz_dir}/include"]
    include_dirs += glob('src/SZ_extra/')
    include_dirs += get_clib_config('zlib', 'include_dirs')
    include_dirs += get_clib_config('zstd', 'include_dirs')

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5sz",
        sources=glob(f"{h5zsz_dir}/src/*.c") + glob(f"{sz_dir}/src/*.c"),
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        libraries=get_clib_config('zlib', 'libraries') + get_clib_config('zstd', 'libraries'),
        extra_objects=get_clib_config('zstd', 'extra_objects'),
    )


PLUGIN_LIB_DEPENDENCIES['sz'] = ('zlib', 'zstd')


def _get_sz3_plugin():
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
    include_dirs += get_clib_config('zstd', 'include_dirs')

    extra_compile_args = ['-std=c++14', '-O3', '-ffast-math', '-fopenmp']
    extra_compile_args += ['/Ox', '/fp:fast', '/openmp']
    extra_link_args = ['-fopenmp', "-lm"]

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5sz3",
        sources=[f"{h5z_sz3_dir}/src/H5Z_SZ3.cpp"],
        extra_objects=get_clib_config('zstd', 'extra_objects'),
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args + get_clib_config('zstd', 'extra_link_args'),
        libraries=get_clib_config('zstd', 'libraries'),
        cpp11_required=True,
    )


PLUGIN_LIB_DEPENDENCIES['sz3'] = ('zstd',)


def _get_sperr_plugin():
    h5z_sperr_dir = "src/H5Z-SPERR"

    return HDF5PluginExtension(
        "hdf5plugin.plugins.libh5sperr",
        sources=prefix(
            f"{h5z_sperr_dir}/src",
            [
                "h5z-sperr.c",
                "icecream.c",
                "compactor.c",
            ]
        ),
        include_dirs=get_clib_config("sperr", "include_dirs") + [f"{h5z_sperr_dir}/include"],
        extra_link_args=['-lstdc++'] + get_clib_config("sperr", "extra_link_args"),
        libraries=get_clib_config("sperr", "libraries"),
        define_macros=get_clib_config("sperr", "macros"),
        cpp20_required=True,
    )


PLUGIN_LIB_DEPENDENCIES['sperr'] = ("sperr", "sperr_filter")


_EMBEDDED_PLUGIN_EXTENSIONS = {
    "blosc": _get_blosc_plugin,
    "blosc2": _get_blosc2_plugin,
    "bshuf": _get_bitshuffle_plugin,
    "bzip2": _get_bzip2_plugin,
    "fcidecomp": _get_fcidecomp_plugin,
    "lz4": _get_lz4_plugin,
    "sperr": _get_sperr_plugin,
    "sz": _get_sz_plugin,
    "sz3": _get_sz3_plugin,
    "zfp": _get_h5zfp_plugin,
    "zstd": _get_zstandard_plugin,
}

PLUGIN_NAMES = set(_EMBEDDED_PLUGIN_EXTENSIONS.keys())


def get_libraries_and_extensions():
    """Returns lists of static libraries and extensions to build.

    Strip libraries and extensions according to HDF5PLUGIN_STRIP env. var.
    """
    stripped_filters = set(
        name.strip().lower()
        for name in os.environ.get('HDF5PLUGIN_STRIP', '').split(',')
        if name.strip()
    )

    if 'all' in stripped_filters:
        return [], []

    if not stripped_filters <= PLUGIN_NAMES:
        raise ValueError(f"Unexpected names in HDF5PLUGIN_STRIP: {stripped_filters - PLUGIN_NAMES}")

    # Filter out library that won't be used because of stripped filters
    lib_names = set(
        itertools.chain.from_iterable(
            lib_names for filter_name, lib_names in PLUGIN_LIB_DEPENDENCIES.items()
            if filter_name not in stripped_filters
        )
    )

    # Filter-out used system libraries
    # Add a prefix to library names to prevent the compiler to use the dynamic library if it exists
    libraries = [
        (f"hdf5plugin_static_clib_{name}", get_clib_config(name))
        for name in lib_names if name not in get_system_clib_names()
    ]

    # Create Python extensions
    embedded_extension_names = PLUGIN_NAMES - stripped_filters
    extensions = [_EMBEDDED_PLUGIN_EXTENSIONS[name]() for name in embedded_extension_names]

    if extensions and not sys.platform == 'win32':
        # Add hdf5 dynamic loading lib
        libraries.append(
            (
                'hdf5_dl',
                {
                    'sources': ['src/hdf5_dl.c'],
                    'include_dirs': BuildConfig.get_hdf5_include_dirs(),
                    'macros': [('H5_USE_18_API', None)],
                    'cflags': [],
                }
            )
        )

    return libraries, extensions


if __name__ == "__main__":
    libraries, extensions = get_libraries_and_extensions()
    setup(
        cmdclass=dict(
            bdist_wheel=BDistWheel,
            build=Build,
            build_clib=BuildCLib,
            build_ext=PluginBuildExt,
            build_py=BuildPy,
        ),
        ext_modules=extensions,
        libraries=libraries,
    )
