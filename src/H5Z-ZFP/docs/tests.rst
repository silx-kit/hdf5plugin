==================
Tests and Examples
==================

The tests directory contains a few simple tests of the H5Z-ZFP_ filter.

The test client, `test_write.c <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c>`_
is compiled a couple of different ways.
One target is ``test_write_plugin`` which demonstrates the use of this filter as
a standalone plugin. The other target, ``test_write_lib``, demonstrates the use
of the filter as an explicitly linked library. These test a simple 1D array with
and without ZFP_ compression using either the :ref:`generic-interface` (for plugin)
or the :ref:`properties-interface` (for library).  You can use the code there as an
example of using the ZFP_ filter either as a plugin or as a library.
The command ``test_write_lib help`` or ``test_write_plugin help`` will print a
list of the example's options and how to use them.

------------------
Write Test Options
------------------

::

    ./test/test_write_lib --help
        ifile=""                                  set input filename
        ofile="test_zfp.h5"                      set output filename
    
    1D dataset generation arguments...
        npoints=1024             set number of points for 1D dataset
        noise=0.001         set amount of random noise in 1D dataset
        amp=17.7             set amplitude of sinusoid in 1D dataset
        chunk=256                      set chunk size for 1D dataset
        doint=0                              also do integer 1D data
    
    ZFP compression paramaters...
        zfpmode=3        (1=rate,2=prec,3=acc,4=expert,5=reversible)
        rate=4                      set rate for rate mode of filter
        acc=0               set accuracy for accuracy mode of filter
        prec=11       set precision for precision mode of zfp filter
        minbits=0          set minbits for expert mode of zfp filter
        maxbits=4171       set maxbits for expert mode of zfp filter
        maxprec=64         set maxprec for expert mode of zfp filter
        minexp=-1074        set minexp for expert mode of zfp filter
                            
    Advanced cases...
        highd=0                                          run 4D case
        sixd=0          run 6D extendable case (requires ZFP>=0.5.4)
        help=0                                     this help message

The test normally just tests compression of 1D array of integer
and double precision data of a sinusoidal array with a small
amount of additive random noise. The ``highd`` test runs a test
on a 4D dataset where two of the 4 dimensions are not correlated.
This tests the plugin's ability to properly set chunking for
HDF5 such that chunks span **only** correlated dimensions and
have non-unity sizes in 3 or fewer dimensions. The ``sixd``
test runs a test on a 6D, extendible dataset representing an
example of using ZFP_ for compression along the *time* axis.

There is a companion, `test_read.c <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_read.c>`_
which is compiled into ``test_read_plugin``
and ``test_read_lib`` which demonstrates use of the filter reading data as a
plugin or library. Also, the commands ``test_read_lib help`` and
``test_read_plugin help`` will print a list of the command line options.

To use the plugin examples, you need to tell the HDF5_ library where to find the
H5Z-ZFP_ plugin with the ``HDF5_PLUGIN_PATH`` environment variable. The value you
pass is the path to the directory containing the plugin shared library.

Finally, there is a Fortran test example,
`test_rw_fortran.F90 <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_rw_fortran.F90>`_.
The Fortran test writes and reads a 2D dataset. However, the Fortran test is designed to
use the filter **only** as a library and not as a plugin. The reason for this is
that the filter controls involve passing combinations of integer and floating 
point data from Fortran callers and this can be done only through the
:ref:`properties-interface`, which by its nature requires any Fortran application
to have to link with an implementation of that interface. Since we need to link
extra code for Fortran, we may as well also link to the filter itself alleviating
the need to use the filter as a plugin. Also, if you want to use Fortran support,
the HDF5_ library must have, of course, been configured and built with Fortran support
as well.

In addition, a number tests are performed in the Makefile which test the plugin
by using some of the HDF5_ tools such as ``h5dump`` and ``h5repack``. Again, to
use these tools to read data compressed with the H5Z-ZFP_ filter, you will need
to inform the HDF5_ library where to find the filter plugin. For example..

::

    env HDF5_PLUGIN_PATH=<dir> h5ls test_zfp.h5

Where ``<dir>`` is the relative or absolute path to a directory containing the
filter plugin shared library.
