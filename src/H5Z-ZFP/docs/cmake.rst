=====
CMake
=====

It is possible to build the H5Z-ZFP filter using the CMake build system. If you decide to
do so, please build ZFP also with its CMake build system. This is necessary to get the 
correct dependencies from ZFP. For example, it is possible to build ZFP with OpenMP support.
The resulting CMake config files of ZFP build will make sure that this OpenMP dependency is
correctly propagated to the build of H5Z-ZFP filter. However, for HDF5 it is not
necessary to build it with its CMake build system but it is strongly recommended.

-----------------
Compiling H5Z-ZFP
-----------------

Similar as for the Makefile installation, the CMake build system is designed such it
compiles both the standalone HDF5_ *plugin* and a separate *library* an application
can explicitly link. See :ref:`plugin-vs-library`

Once you have installed both HDF5 and ZFP, you can compile H5Z-ZFP_ using a command=line...

::

    export HDF_DIR=<path-to_hdf5>
    export ZFP_DIR=<path-to-zfp>
    CC=<C-compiler> FC=<Fortran-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> <src-dir>

where ``<path-to-zfp>`` is a directory containing ZFP_ ``inc[lude]`` and ``lib`` directories and
``<path-to-hdf5>`` is a directory containing HDF5_ ``include`` and ``lib`` directories. Furthermore,
``src-dir`` is the directory where the source is located and ``path-to-install`` is the directory in 
which the resulting *plugin* and *library* will be installed. Once cmake has finished successfully, 
you can build and install the filter using the command...

::

    make install

This cmake and make combination builds both the C and Fortran interface. In the case you want to specify
the ``<path-to-hdf5>`` and ``<path-to-zfp>>`` via command-line to CMake, the command looks like this...

::

    CC=<C-compiler> FC=<Fortran-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> 
        -DCMAKE_PREFIX_PATH="<path-tohdf5>;<path-to-zfp>" <src-dir>

Please, notice the double quotes in the CMAKE_PREFIX_PATH expression. These are needed to make sure that semicolon
is interpreted as a semicolon instead of a new command.

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

Suppose you have built the H5Z-ZFP filter using the CMake build system and installed it in ``<path-to-h5z_zfp>``.
To include it in another CMake project is done using the following steps. First edit the ``CMakeLists.txt``
by adding the following two lines...

::

   cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or IMPORTED target.
   ...
   set(H5Z_ZFP_USE_STATIC_LIBS OFF)
   find_package(H5Z_ZFP 1.0.1 CONFIG)
   ...
   target_link_libraries(<target> h5z_zfp::h5z_zfp)
   ...

where ``<target>`` in the target within the CMake project. This could be, for example, a executable or library.
Furthermore, check if the cmake version is equal or greater than 3.9. Next, you need to make sure that the filter 
can be found by CMake, followed CMake itself and make...

::

   export H5Z_ZFP_DIR=<path-to-h5z_zfp>
   CC=<C-compiler> cmake -DCMAKE_INSTALL_PREFIX=<path-to-install> <src-dir>
   make install

The cmake command itself could be different depending on the CMake project you have created. If you want
to make use of the H5Z_ZFP *library* instead of the plugin, change cmake variable H5Z_ZFP_USE_STATIC_LIBS 
to ON and build the project.
