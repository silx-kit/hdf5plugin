============
Installation
============

------------------------------------------
Installing via `Spack <https://spack.io>`_
------------------------------------------
The HDF5_ and ZFP_ libraries and the H5Z-ZFP_ plugin are all now part of the
Spack_ package manager. If you already have Spack_ installed, the easiest way to
install H5Z-ZFP_ is to simply use the Spack_ command ``spack install h5z-zfp``.
If you do not have Spack_ installed, it is very easy to install.

::

    git clone https://github.com/llnl/spack.git
    . spack/share/spack/setup-env.sh
    spack install h5z-zfp

By default, H5Z-ZFP_ will attempt to build with Fortran support which requires
a Fortran compiler. If you wish to exclude support for Fortran, use the command

::

    spack install h5z-zfp~fortran

Note that these commands will build H5Z-ZFP_ **and** all of its dependencies including
the HDF5_ library (as well as a number of other dependencies you may not initially
expect. Be patient and let the build complete). In addition, by default, Spack_ installs
packages to directory *hashes* *within* the cloned Spack_ repository's directory tree,
``$spack/opt/spack``. You can find the resulting installed HDF5_ library with the command
``spack find -vp hdf5`` and your resulting H5Z-ZFP plugin installation with the command
``spack find -vp h5z-zfp``. If you wish to exercise more control over where Spack_ 
installs things, have a look at
`configuring Spack <https://spack.readthedocs.io/en/latest/config_yaml.html#install-tree>`_

-------------------
Manual Installation
-------------------

If Spack_ is not an option for you, information on *manually* installing is provided
here.

^^^^^^^^^^^^^
Prerequisites
^^^^^^^^^^^^^

* `ZFP Library <http://computation.llnl.gov/projects/floating-point-compression/download/zfp-0.5.0.tar.gz>`_ (or from `Github <https://github.com/LLNL/zfp>`_)
* `HDF5 Library <https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.17.tar.gz>`_
* `H5Z-ZFP filter plugin <https://github.com/LLNL/H5Z-ZFP>`_

^^^^^^^^^^^^^
Compiling ZFP
^^^^^^^^^^^^^

* There is a ``Config`` file in top-level directory of the ZFP_ distribution that holds ``make`` variables
  the ZFP_ Makefiles use. By default, this file is setup for a vanilla GNU compiler. If this is not the
  appropriate compiler, edit ``Config`` as necessary to adjust the compiler and compilation flags.
* An important flag you **will** need to adjust in order to use the ZFP_ library with this HDF5_ filter is
  the ``BIT_STREAM_WORD_TYPE`` CPP flag. To use ZFP_ with H5Z-ZFP_, the ZFP_ library **must** be compiled
  with ``BIT_STREAM_WORD_TYPE`` of ``uint8``. Typically, this is achieved by including a line in ``Config``
  of the form ``DEFS += -DBIT_STREAM_WORD_TYPE=uint8``. If you attempt to use this filter with a ZFP_
  library compiled  differently from this, the  filter's ``can_apply`` method will always return
  false. This will result in silently ignoring an HDF5_ client's  request to compress data with
  ZFP_. Also, be sure to see :ref:`endian-issues`.
* After you have setup ``Config``, simply run ``make`` and it will build the ZFP_ library placing
  the library in a ``lib`` sub-directory and the necessary include files in ``inc[lude]`` sub-directory.
* For more information and details, please see the `ZFP README <https://github.com/LLNL/zfp/blob/master/README.md>`_.

^^^^^^^^^^^^^^^
Compiling HDF5_
^^^^^^^^^^^^^^^

* If you want to be able to run the fortran tests for this filter, HDF5_ must be
  configured with *both* the ``--enable-fortran`` and ``--enable-fortran2003``
  configuration switches. Otherwise, any vanilla installation of HDF5_ is acceptable.
  
* The Fortran interface to this filter *requires* a Fortran 2003 compiler
  because it uses
  `ISO_C_BINDING <https://gcc.gnu.org/onlinedocs/gfortran/ISO_005fC_005fBINDING.html>`_
  to define the Fortran interface.

-----------------
Compiling H5Z-ZFP
-----------------

H5Z-ZFP_ is designed to be compiled both as a standalone HDF5_ *plugin* and as a separate
*library* an application can explicitly link. See :ref:`plugin-vs-library`.

Once you have installed the prerequisites, you can compile H5Z-ZFP_ using a command-line...

::

    make [FC=<Fortran-compiler>] CC=<C-compiler>
        ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5>
        PREFIX=<path-to-install>

where ``<path-to-zfp>`` is a directory containing ZFP_ ``inc[lude]`` and ``lib`` dirs and
``<path-to-hdf5>`` is a directory containing HDF5_ ``include`` and ``lib`` dirs.
If you don't specify a C compiler, it will try to guess one from your path. Fortran
compilation is optional. If you do not specify a Fortran compiler, it will not attempt
to build the Fortran interface. However, if the variable ``FC`` is already defined in
your enviornment (as in Spack_ for example), then H5Z-ZFP_ will attempt to build Fortran.
If this is not desired, the solution is to pass an *empty* ``FC`` on the make command
line as in...

::

    make FC= CC=<C-compiler>
        ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5>
        PREFIX=<path-to-install>


The Makefile uses  GNU Make syntax and is designed to  work on OSX and
Linux. The filter has been tested on gcc, clang, xlc, icc and pgcc  compilers
and checked with valgrind.

The command ``make help`` will print useful information
about various make targets and variables. ``make check`` will compile everything
and run a handful of tests.

If you don't specify a ``PREFIX``, it will install to ``./install``. The installed
package will look like...

::

    $(PREFIX)/include/{H5Zzfp.h,H5Zzfp_plugin.h,H5Zzfp_props.h,H5Zzfp_lib.h}
    $(PREFIX)/plugin/libh5zzfp.{so,dylib}
    $(PREFIX)/lib/libh5zzfp.a

where ``$(PREFIX)`` resolves to whatever the full path of the installation is.

To use the installed filter as an HDF5_ *plugin*, you would specify, for example,
``setenv HDF5_PLUGIN_PATH $(PREFIX)/plugin``

--------------------------------
H5Z-ZFP Source Code Organization
--------------------------------

The source code is in two separate directories

    * ``src`` includes the ZFP_ filter and a few header files

        * ``H5Zzfp_plugin.h`` is an optional header file applications *may* wish
          to include because it contains several convenient macros for easily
          controlling various compression modes of the ZFP_ library (*rate*,
          *precision*, *accuracy*, *expert*) via the :ref:`generic-interface`. 
        * ``H5Zzfp_props.h`` is a header file that contains functions to control the
          filter using *temporary* :ref:`properties-interface`. Fortran callers are
          *required* to use this interface.
        * ``H5Zzfp_lib.h`` is a header file for applications that wish to use the filter
          explicitly as a library rather than a plugin.
        * ``H5Zzfp.h`` is an *all-of-the-above* header file for applications that don't
          care too much about separating out the above functionalities.

    * ``test`` includes various tests. In particular ``test_write.c`` includes examples
      of using both the :ref:`generic-interface` and :ref:`properties-interface`. In 
      addition, there is an example of how to use the filter from Fortran in ``test_rw_fortran.F90``.

----------------
Silo Integration
----------------

This filter is also built-in to the `Silo library <https://wci.llnl.gov/simulation/computer-codes/silo>`_.
In particular, the ZFP_ library
itself is also embedded in Silo but is protected from appearing in Silo's
global namespace through a struct of function pointers (see `Namespaces in C) <https://visitbugs.ornl.gov/projects/silo/wiki/Using_C_structs_as_a_kind_of_namespace_mechanism_to_reduce_global_symbol_bloat>`_.
If you happen to examine the source code for H5Z-ZFP_, you will see some logic there
that is specific to using this plugin within Silo and dealing with ZFP_ as an embedded
library using this struct of function pointers wrapper. Just ignore this.
