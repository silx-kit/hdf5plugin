==================================
Using H5Z-ZFP Plugin with H5Repack
==================================
A convenient way to use and play with the ZFP_ filter is a *plugin* with
the HDF5_ `h5repack <https://support.hdfgroup.org/HDF5/doc/RM/Tools.html#Tools-Repack>`_
utility using the ``-f`` filter argument to apply ZFP to existing data in a file.

-----------------
Patching h5repack
-----------------
Some versions of HDF5_'s ``h5repack`` utility contain a bug that prevents
proper parsing of the ``-f`` argument's option. In order to use ``h5repack``
with ``-f`` argument as described here, you need to apply the patch from
`h5repack_parse.patch <https://github.com/LLNL/H5Z-ZFP/blob/master/test/h5repack_parse.patch>`_.
To do so, after you've downloaded and untar'd HDF5_ but before you've built
it, do something like the following using HDF5-1.8.14 as an example::

    gunzip < hdf5-1.8.14.tar.gz | tar xvf -
    cd hdf5-1.8.14
    patch ./tools/h5repack/h5repack_parse.c <path-to-H5Z-ZFP-test-dir>/h5repack_parse.patch

-------------------------------------
Constructing an HDF5_ cd_values array
-------------------------------------
HDF5_'s ``h5repack`` utility uses only the *generic* interface to HDF5_ filters.
Another challenge in using ``h5repack`` as described here is constructing the set
``unsigned int cd_values`` as is used in
`H5Pset_filter() <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFilter>`_
required by the *generic* HDF5_ filter interface, especially because
of the type-punning (doubles as unsigned int) involved.

**Note:** Querying an existing dataset using ``h5dump`` or ``h5ls`` to obtain
the *cd_values* stored with a ZFP_ compressed dataset
**will not provide the correct cd_values**. This is because the *cd_values*
stored in the file are different from those used in the *generic* interface
to invoke the ZFP_ filter.

To facilitate constructing a valid ``-f`` argument to ``h5repack``, we have
created a utility program, ``print_h5repack_farg``, which is presently in the
``test`` directory and is built when tests are built.

You can use the ``print_h5repack_farg`` utility to read a command-line
consisting of ZFP_ filter parameters you wish to use and it will output
part of the command-line needed for the ``-f`` argument to ``h5repack``.

--------
Examples
--------

In the examples below, we use ``h5repack`` with the example data file,
``mesh.h5`` in the tests directory.

To use ZFP_ filter in *rate* mode with a rate of ``4.5`` bits per value,
first, use the ``print_h5repack_farg``::

    % ./print_h5repack_farg zfpmode=1 rate=4.5
    
    Print cdvals for set of ZFP compression paramaters...
        zfpmode=1        set zfp mode (1=rate,2=prec,3=acc,4=expert)
        rate=4.5                    set rate for rate mode of filter
        acc=0               set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
        help=0                                     this help message

    h5repack -f argument...
        -f UD=32013,4,1,0,0,1074921472

Next, cut-n-paste the ``-f UD=32013,4,1,0,0,1074921472`` in a command
to ``h5repack`` like so::

    env LD_LIBRARY_PATH=<path-to-dir-with-libhdf5.so>:$(LD_LIBRARY_PATH) \
        HDF5_PLUGIN_PATH=<path-to-dir-with-libh5zzfp.so> \
        $(HDF5_BIN)/h5repack -f UD=32013,4,1,0,0,1074921472 \
             -l Pressure,Pressure2,Pressure3:CHUNK=10x20x5 \
             -l Velocity,Velocity2,Velocity3,VelocityZ,VelocityZ2,VelocityZ3:CHUNK=11x21x1x1 \
             -l VelocityX_2D:CHUNK=21x31 \
             mesh.h5 mesh_repack.h5

where the ``-l`` arguments indicate the dataset(s) to be re-packed as well
as their (new) chunking.

To use ZFP_ filter in *accuracy* mode with an accuracy of ``0.075``,
first, use the ``print_h5repack_farg``::

    % ./print_h5repack_farg zfpmode=3 acc=0.075
    
    Print cdvals for set of ZFP compression paramaters...
        zfpmode=3        set zfp mode (1=rate,2=prec,3=acc,4=expert)
        rate=4                      set rate for rate mode of filter
        acc=0.075           set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
        help=0                                     this help message

    h5repack -f argument...
        -f UD=32013,4,3,0,858993459,1068708659

Next, cut-n-paste the ``-f UD=32013,4,3,0,858993459,1068708659`` in a command
to ``h5repack`` like so::

    env LD_LIBRARY_PATH=<path-to-dir-with-libhdf5.so>:$(LD_LIBRARY_PATH) \
        HDF5_PLUGIN_PATH=<path-to-dir-with-libh5zzfp.so> \
        $(HDF5_BIN)/h5repack -f UD=32013,4,3,0,858993459,1068708659 \
             -l Pressure,Pressure2,Pressure3:CHUNK=10x20x5 \
             -l Velocity,Velocity2,Velocity3,VelocityZ,VelocityZ2,VelocityZ3:CHUNK=11x21x1x1 \
             -l VelocityX_2D:CHUNK=21x31 \
             mesh.h5 mesh_repack.h5
