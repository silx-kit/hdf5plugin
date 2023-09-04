============
Installation
============

Three ways to install H5Z-ZFP_ are provided.
These are

* With generic (GNU) :ref:`make <gnumake>`
* With :ref:`CMake <ceemake>`
* With :ref:`Spack <spack2>`

For both generic make and CMake_, you are responsible for also installing (or knowing where the installations are) the dependencies, ZFP_ and HDF5_.
For Spack_ installations, Spack_ will handle installation of dependencies as well.

.. _gnumake:

---------------------------------
Installing via Generic (GNU) Make
---------------------------------

H5Z-ZFP_ installation supports both vanilla (`GNU <https://www.gnu.org/software/make/>`__) Make (described below) as well as :ref:`CMake <ceemake>`.

^^^^^^^^^^^^^
Prerequisites
^^^^^^^^^^^^^

* `ZFP Library <https://github.com/LLNL/zfp/releases>`_ (or from `Github <https://github.com/LLNL/zfp>`_)
* `HDF5 Library <https://portal.hdfgroup.org/display/support/Downloads>`_
* `H5Z-ZFP filter plugin <https://github.com/LLNL/H5Z-ZFP>`_

.. _zfp-config:

^^^^^^^^^^^^^^
Compiling ZFP_
^^^^^^^^^^^^^^

* There is a ``Config`` file in top-level directory of the ZFP_ distribution that holds ``make`` variables the ZFP_ Makefiles use. By default, this file is setup for a vanilla GNU compiler.
  If this is not the appropriate compiler, edit ``Config`` as necessary to adjust the compiler and compilation flags.
* An important flag you **will** need to adjust in order to use the ZFP_ library with this HDF5_ filter is the ``BIT_STREAM_WORD_TYPE`` CPP flag.
  To use ZFP_ with H5Z-ZFP_, the ZFP_ library **must** be compiled with ``BIT_STREAM_WORD_TYPE`` of ``uint8``.
  Typically, this is achieved by including a line in ``Config`` of the form ``DEFS += -DBIT_STREAM_WORD_TYPE=uint8``.
  If you attempt to use this filter with a ZFP_ library compiled  differently from this, the  filter's ``can_apply`` method will always return false.
  This will result in silently ignoring an HDF5_ client's  request to compress data with ZFP_.
  Also, be sure to see :ref:`endian-issues`.
* After you have setup ``Config``, simply run ``make`` and it will build the ZFP_ library placing the library in a ``lib`` sub-directory and the necessary include files in ``inc[lude]`` sub-directory.
* For more information and details, please see the `ZFP README <https://github.com/LLNL/zfp/blob/master/README.md>`_.

^^^^^^^^^^^^^^^
Compiling HDF5_
^^^^^^^^^^^^^^^

* If you want to be able to run the fortran tests for this filter, HDF5_ must be configured with *both* the ``--enable-fortran`` and ``--enable-fortran2003`` configuration switches.
  Otherwise, any vanilla installation of HDF5_ is acceptable.
  
* The Fortran interface to this filter *requires* a Fortran 2003 compiler because it uses `ISO_C_BINDING <https://gcc.gnu.org/onlinedocs/gfortran/ISO_005fC_005fBINDING.html>`_ to define the Fortran interface.

* If you are using HDF5-1.12 and wish to use the filter as a *library* (see :ref:`plugin-vs-library`), you may need configure HDF5 with ``--disable-memory-alloc-sanity-check`` to work around a memory management issue in HDF5.

^^^^^^^^^^^^^^^^^
Compiling H5Z-ZFP
^^^^^^^^^^^^^^^^^

H5Z-ZFP_ is designed to be compiled both as a standalone HDF5_ *plugin* and as a separate *library* an application can explicitly link. See :ref:`plugin-vs-library`.

Once you have installed the prerequisites, you can compile H5Z-ZFP_ using a command-line...

::

    make [FC=<Fortran-compiler>] CC=<C-compiler> \
        ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5> \
        PREFIX=<path-to-install>

where ``<path-to-zfp>`` is a directory containing ZFP_ ``inc[lude]`` and ``lib`` dirs and ``<path-to-hdf5>`` is a directory containing HDF5_ ``include`` and ``lib`` dirs.
If you don't specify a C compiler, it will try to guess one from your path.
Fortran compilation is optional.
If you do not specify a Fortran compiler, it will not attempt to build the Fortran interface.
However, if the variable ``FC`` is already defined in your environment (as in Spack_ for example), then H5Z-ZFP_ will attempt to build Fortran.
If this is not desired, the solution is to pass an *empty* ``FC`` on the make command line as in...

::

    make FC= CC=<C-compiler> \
        ZFP_HOME=<path-to-zfp> HDF5_HOME=<path-to-hdf5> \
        PREFIX=<path-to-install>


The Makefile uses  GNU Make syntax and is designed to  work on OSX and Linux. The filter has been tested on gcc, clang, xlc, icc and pgcc compilers and checked with valgrind.

The command ``make help`` will print useful information about various make targets and variables. ``make check`` will compile everything and run a handful of tests.

If you don't specify a ``PREFIX``, it will install to ``./install``.
The installed package will look like...

::

    $(PREFIX)/include/{H5Zzfp.h,H5Zzfp_plugin.h,H5Zzfp_props.h,H5Zzfp_lib.h}
    $(PREFIX)/plugin/libh5zzfp.{so,dylib}
    $(PREFIX)/lib/libh5zzfp.a

where ``$(PREFIX)`` resolves to whatever the full path of the installation is.

To use the installed filter as an HDF5_ *plugin*, you would specify, for example,
``setenv HDF5_PLUGIN_PATH $(PREFIX)/plugin``

.. _ceemake:

--------------------
Installing via CMake
--------------------

It is possible to build the H5Z-ZFP_ filter using the CMake_ build system.
To use CMake_ for H5Z-ZFP_, it is necessary to have also built ZFP_ with CMake.
This is necessary to get the correct dependencies from ZFP_.
For example, it is possible to build ZFP_ with OpenMP support.
The resulting CMake_ config files of ZFP_ build will make sure that this OpenMP dependency is correctly propagated to the build of H5Z-ZFP_ filter.
However, for HDF5_ it is not necessary to build it with its CMake_ build system but it is strongly recommended.

ZFP_ must have been :ref:`configured <zfp-config>` with ``BIT_STREAM_WORD_TYPE`` of ``uint8`` as described above.

Similar as for the Makefile installation, the CMake_ build system is designed such it compiles both the standalone HDF5_ *plugin* and a separate *library* an application can explicitly link. See :ref:`plugin-vs-library`

Once both HDF5_ and ZFP_ have been installed, H5Z-ZFP_ can be compiled using a command=line...

::

    export HDF5_DIR=<path-to_hdf5>
    export ZFP_DIR=<path-to-zfp-config>
    CC=<C-compiler> FC=<Fortran-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> <src-dir>

where ``<path-to-zfp-config>`` is a directory containing ``zfp-config.cmake`` and ``<path-to-hdf5>`` is a directory containing HDF5_ ``include`` and ``lib`` directories.
Furthermore, ``src-dir`` is the directory where the H5Z-ZFP_ source is located and ``path-to-install`` is the directory in which the resulting *plugin* and *library* will be installed.
Once ``cmake`` has finished successfully, you can build and install the filter using the command...

::

    make install

This ``cmake`` and ``make`` combination builds both the C and Fortran interface.
In the case you want to specify the ``<path-to-hdf5>`` and ``<path-to-zfp>>`` via command-line to CMake_, the command looks like this...

::

    CC=<C-compiler> FC=<Fortran-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> 
        -DCMAKE_PREFIX_PATH="<path-tohdf5>;<path-to-zfp>" <src-dir>

.. note::

   The double quotes in the CMAKE_PREFIX_PATH expression are necessary to make sure that semicolon is interpreted as a semicolon instead of a new command.

It is possible to build the filter without the Fortran interface. This is done as follows...

::

    export HDF5_DIR=<path-to_hdf5>
    export ZFP_DIR=<path-to-zfp>
    CC=<C-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> -DFORTRAN_INTERFACE:BOOL=OFF <src-dir>

followed by the same make command...

::

    make install

-------------------------------------------
Including H5Z-ZFP filter in a CMake project
-------------------------------------------

Suppose you have built the H5Z-ZFP_ filter using the CMake_ build system and installed it in ``<path-to-h5z_zfp>``.
To include it in another CMake_ project is done using the following steps. First edit the ``CMakeLists.txt``
by adding the following two lines...

::

   cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or IMPORTED target.
   ...
   set(H5Z_ZFP_USE_STATIC_LIBS OFF)
   find_package(H5Z_ZFP 1.0.1 CONFIG)
   ...
   target_link_libraries(<target> h5z_zfp::h5z_zfp)
   ...

where ``<target>`` in the target within the CMake_ project.
This could be, for example, an executable or library.
Furthermore, check if the ``cmake`` version is equal or greater than 3.9.
Next, you need to make sure that the filter can be found by CMake_, followed by ``cmake`` itself and ``make``...

::

   export H5Z_ZFP_DIR=<path-to-h5z_zfp>
   CC=<C-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> <src-dir>
   make install

The ``cmake`` command itself could be different depending on the CMake_ project you have created.
If you want to make use of the H5Z-ZFP_ *library* instead of the plugin, change cmake variable ``H5Z_ZFP_USE_STATIC_LIBS`` to ``ON`` and build the project.

.. _spack2:

---------------------
Installing via Spack_
---------------------
If you already have experience with Spack_, one way to install H5Z-ZFP_ is to use the command ``spack install h5z-zfp``.
If you do not have Spack_ installed, it is easy to install.
Assuming you are working in a Bash shell...::

    git clone https://github.com/llnl/spack.git
    cd spack
    git checkout releases/v0.20
    . ./share/spack/setup-env.sh
    spack install h5z-zfp

.. note::

   It is important to work from a *released* branch of Spack_.
   The command ``git checkout releases/v0.20`` ensures this.
   If a newer release of Spack_ is available, by all means feel free to use it.
   Just change the ``v0.20`` to indicate the release of the Spack_ you want.
   The command ``git branch -r | grep releases`` will produce a list of the available release branches.

If you are using a version of Spack_ very much older than the release of H5Z-ZFP_ you intend to use, you may have to *pin* various versions of H5Z-ZFP_, ZFP_ and/or HDF5_.
This is done by using Spack_'s ``@`` modifier to specify versions.
For example, to *pin* the version of the ZFP_ library to 0.5.5, the Spack_ command would look like::

    spack install h5z-zfp ^zfp@0.5.5

To use the ``develop`` version of H5Z-ZFP_ with version 1.10.6 of HDF5_ ::

    spack install h5z-zfp@develop ^hdf5@1.10.6

By default, H5Z-ZFP_ will attempt to build with Fortran support which requires a Fortran compiler.
If you wish to exclude support for Fortran, use the command::

    spack install h5z-zfp~fortran

Spack_ packages can sometimes favor the use of dependencies you may not need.
For example, the HDF5_ package favors the use of MPI.
Since H5Z-ZFP_ depends on HDF5_, this behavior will then create a dependency of H5Z-ZFP_ on MPI.
To avoid this, you can force Spack_ to use a version of HDF5_ *without* MPI.
In the example command below, we force Spack_ to not use MPI with HDF5_ and to not use OpenMP with ZFP_::

    spack install h5z-zfp~fortran ^hdf5~mpi~fortran ^zfp~openmp

This can have the effect of substantially reducing the number of dependencies Spack_ winds up having to build (from 35 in one case to 10) in order to install H5Z-ZFP_ which, in turn, speeds up the install process.

.. note::

   Spack_ will build H5Z-ZFP_ **and** all of its dependencies including the HDF5_ library *as well as a number of other dependencies you may not initially expect*.
   Be patient and let the build complete.
   It may take as much as an hour.

In addition, by default, Spack_ installs packages to directory *hashes within* the cloned Spack_ repository's directory tree, ``$spack/opt/spack``.
You can find the resulting installed HDF5_ library with the command ``spack find -vp hdf5`` and the resulting H5Z-ZFP_ plugin installation with the command ``spack find -vp h5z-zfp``.
If you wish to exercise more control over how and where Spack_ installs, have a look at
`configuring Spack <https://spack.readthedocs.io/en/latest/config_yaml.html#install-tree>`_

--------------------------------
H5Z-ZFP Source Code Organization
--------------------------------

The source code is in two separate directories

    * ``src`` includes the ZFP_ filter and a few header files

        * ``H5Zzfp_plugin.h`` is an optional header file applications *may* wish to include because it contains several convenient macros for easily controlling various compression modes of the ZFP_ library (*rate*, *precision*, *accuracy*, *expert*) via the :ref:`generic-interface`. 
        * ``H5Zzfp_props.h`` is a header file that contains functions to control the filter using *temporary* :ref:`properties-interface`.
          Fortran callers are *required* to use this interface.
        * ``H5Zzfp_lib.h`` is a header file for applications that wish to use the filter explicitly as a library rather than a plugin.
        * ``H5Zzfp.h`` is an *all-of-the-above* header file for applications that don't care too much about separating out the above functionalities.

    * ``test`` includes various tests. In particular ``test_write.c`` includes examples of using both the :ref:`generic-interface` and :ref:`properties-interface`.
      In addition, there is an example of how to use the filter from Fortran in ``test_rw_fortran.F90``.

----------------
Silo Integration
----------------

This filter (``H5Zzfp.c``) is also built-in to the `Silo library <https://wci.llnl.gov/simulation/computer-codes/silo>`_.
In particular, the ZFP_ library itself is also embedded in Silo but is protected from appearing in Silo's global namespace through a struct of function pointers (see `Namespaces in C <https://github.com/markcmiller86/silo-issues/wiki/Using-C-structs-as-a-kind-of-namespace-mechanism-to-reduce-global-symbol-bloat>`_).
If you happen to examine the source code here for H5Z-ZFP_, you will see some logic here that is specific to using this plugin within Silo and dealing with ZFP_ as an embedded library using this struct of function pointers wrapper.
In the source code for H5Z-ZFP_ this manifests as something like what is shown in the code snippet below... 

.. literalinclude:: ../src/H5Zzfp.c
   :language: c
   :linenos:
   :start-after: /* set up dummy zfp field to compute meta header */
   :end-before:  if (!dummy_field)
   
In the code snippet above, note the funny ``Z`` in front of calls to various methods in the ZFP_ library.
When compiling H5Z-ZFP_ normally, that ``Z`` normally resolves to the empty string.
But, when the code is compiled with ``-DAS_SILO_BUILTIN`` (which is supported and should be done *only* when ``H5Zzfp.c`` is being compiled *within* the Silo library and *next to* a version of ZFP_ that is embedded in Silo) that ``Z`` resolves to the name of a struct and struct-member dereferencing operator as in ``zfp.``.
There is a similar ``B`` used for a similar purpose ahead of calls to ZFP_'s bitstream library.
This is something to be aware of and to adhere to if you plan to contribute any code changes here.
