==========
Interfaces
==========

There  are two  interfaces  to  control the  filter.  One uses  HDF5_'s
*generic* interface via  an array of ``unsigned int cd_values`` as is used
in `H5Pset_filter() <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFilter>`_. The other
uses HDF5_ `properties <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#GenericPropFuncs>`_ 
added to the `dataset creation property list <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#DatasetCreatePropFuncs>`_
used when the dataset to be compressed is being created. You  can find examples  of writing
HDF5_ data using both the
`generic <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c#L263>`_ 
and
`properties <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c#L290>`__
interfaces in
`test_write.c <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c>`_.

The filter itself supports either interface. The filter also supports all of the
standard ZFP_ controls for affecting compression including *rate*, *precision*,
*accuracy*, *expert* and *reversible* modes. For more information and details about
these modes of controlling ZFP_ compression, please see the
`ZFP README <https://github.com/LLNL/zfp/blob/master/README.md>`_.

Finally, you should *not* attempt to combine the ZFP_ filter with any other
*byte order altering* filter such as, for example, HDF5_'s shuffle filter.
Space-performance will be ruined. This is in contrast to HDF5_'s
`deflate <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetDeflate>`_
filter which often performs *better* when used in conjunction with the
`shuffle <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetShuffle>`_
filter.

.. _generic-interface:

-----------------
Generic Interface
-----------------

The generic interface is the only means of controlling the H5Z-ZFP_ filter when it
is used as a
`dynamically loaded HDF5 plugin <https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf>`_.

For the generic interface, the following CPP macros  are defined in
the ``H5Zzfp_plugin.h`` header file::

    H5Pset_zfp_rate_cdata(double rate, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_precision_cdata(unsigned int prec, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_accuracy_cdata(double acc, size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_expert_cdata(unsigned int minbits, unsigned int maxbits,
                            unsigned int maxprec, int minexp,
                            size_t cd_nelmts, unsigned int *cd_vals);
    H5Pset_zfp_reversible_cdata(size_t cd_nelmts, unsigned int *cd_vals);

These  macros  utilize *type punning* to store the relevant ZFP_ parameters  into  a
sufficiently large array (>=6) of ``unsigned int cd_values``. It is up to
the  caller to  then call
`H5Pset_filter() <https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFilter>`_
with  the array  of cd_values constructed by one of these macros.

Here is example code from
`test_write.c <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c>`_...

.. literalinclude:: ../test/test_write.c
   :language: c
   :linenos:
   :start-after: setup zfp filter via generic (cd_values) interface
   :end-before: #else

However, these  macros are only a  convenience. You do  not **need** the
``H5Zzfp_plugin.h`` header file if you want  to avoid using it. But, you are then
responsible  for setting  up  the ``cd_values``  array  correctly for  the
filter.  For reference,  the ``cd_values``  array for  this ZFP_  filter is
defined like so...

+-----------+---------------------------------------------------------+
|           |                     cd_values index                     |
+-----------+--------+--------+---------+---------+---------+---------+
| ZFP mode  |     0  |    1   |    2    |    3    |    4    |    5    | 
+-----------+--------+--------+---------+---------+---------+---------+
| rate      |     1  | unused |  rateA  |  rateB  |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| precision |     2  | unused |  prec   |  unused |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| accuracy  |     3  | unused |  accA   |  accB   |  unused |  unused |
+-----------+--------+--------+---------+---------+---------+---------+
| expert    |     4  | unused |  minbits|  maxbits|  maxprec|  minexp |
+-----------+--------+--------+---------+---------+---------+---------+
| reversible|     5  | unused |  unused |  unused |  unused |  unsued |
+-----------+--------+--------+---------+---------+---------+---------+

A/B are high/low 32-bit words of a double.

Note that  the cd_values  used in the generic interface to  ``H5Pset_filter()``
are **not the same** cd_values ultimately stored  to the HDF5_ dataset header
for a compressed dataset. The  values are transformed in the set_local
method to use ZFP_'s internal  routines for 'meta' and 'mode' data. So,
don't make the mistake of examining  the values you find in a file and
think you can use those same  values, for example, in an invokation of
h5repack.

Because of the type punning involved, the generic interface is not
suitable for Fortran callers.

.. _properties-interface:

--------------------
Properties Interface
--------------------

For the properties interface, the following functions are defined in
the ``H5Zzfp_props.h`` header file::

    herr_t H5Pset_zfp_rate(hid_t dcpl_id, double rate);
    herr_t H5Pset_zfp_precision(hid_t dcpl_id, unsigned int prec);
    herr_t H5Pset_zfp_accuracy(hid_t dcpl_id, double acc);
    herr_t H5Pset_zfp_expert(hid_t dcpl_id,
        unsigned int minbits, unsigned int maxbits,
        unsigned int maxprec, int minexp);
    herr_t H5Pset_zfp_reversible(hid_t dcpl_id);

These  functions take a dataset creation property list, ``hid_t dcp_lid`` and
create  temporary HDF5_ property
list entries  to control the  ZFP_ filter. Calling any of these functions
removes the effects of any previous call to any one of these functions.
In addition, calling any one of these functions also has the effect of
adding the filter to the pipeline.

Here is example code from
`test_write.c <https://github.com/LLNL/H5Z-ZFP/blob/master/test/test_write.c>`_...

.. literalinclude:: ../test/test_write.c
   :language: c
   :linenos:
   :start-after: When filter is used as a library, we need to init it
   :end-before: #endif 

The properties interface  is more type-safe than the generic interface.
However, there  is no way for the implementation of the properties interface
to reside within the filter plugin itself. The properties interface requires that the caller link
with  with the filter as a *library*, ``libh5zzfp.a``.  The generic 
interface does not require this.

Note that either interface can be used whether the
filter is used as a plugin or as a library. The difference
is whether the application calls ``H5Z_zfp_initialize()`` or not.

-----------------
Fortran Interface
-----------------

A Fortran interface based on the properties interface, described above,
has been added by Scot Breitenfeld of the HDF5_ group. The code that
implements the Fortran interface is in the file ``H5Zzfp_props_f.F90``.
An example of its use is in ``test/test_rw_fortran.F90``. The properties
interface is the only interface available for Fortran callers.

.. _plugin-vs-library:

----------------------------
Plugin vs. Library Operation
----------------------------

The filter is designed to be compiled for use as both a standalone HDF5_
`dynamically loaded HDF5 plugin <https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf>`_
and as an explicitly linked *library*.
When it is used as a plugin, it is a best practice to link the ZFP_ library
into the plugin dynamic/shared object as a *static* library. Why? In so doing,
we ensure that all ZFP_ public namespace symbols remain *confined* to the plugin
so as not to interfere with any application that may be directly explicitly linking
to the ZFP_ library for other reasons.

All HDF5_ applications are *required*
to *find* the plugin dynamic library (named ``lib*.{so,dylib}``)
in a directory specified by the enviornment
variable, ``HDF5_PLUGIN_PATH``. Currently, the HDF5 library offers
no mechanism for applications themselves to have pre-programmed
paths in which to search for a plugin. Applications are
then always vulnerable to an incorrectly specified or unspecified ``HDF5_PLUGIN_PATH``
environment variable.

However, the plugin can also be used explicitly as a *library*. In this case,
**do** **not** specify the ``HDF5_PLUGIN_PATH`` enviornment variable and instead
have the application link to ``libH5Zzfp.a`` in the ``lib`` dir of the installation.
Instead two initialization and finalization routines are defined::

    int H5Z_zfp_initialize(void);
    int H5Z_zfp_finalize(void);

These functions are defined in the ``H5Zzfp_lib.h`` header file.
Any applications that wish to use the filter as a *library* are required to call
the initialization routine, ``H5Z_zfp_initialize()`` before the filter can be
referenced. In addition, to free up resources used by the filter, applications may
call ``H5Z_zfp_finalize()`` when they are done using the filter.
