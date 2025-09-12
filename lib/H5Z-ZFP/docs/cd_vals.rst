=======================================
H5Z-ZFP and the HDF5 filter's cd_values
=======================================

.. note::
   The details described here are likely relevant only to *developers* of the H5Z-ZFP_ filter.
   If you just want to *use* the filter, you can ignore this material.

The HDF5_ library uses an array of values, named ``cd_values`` in formal arguments documenting various API functions, for managing *auxiliary data* for a filter.
Instances of this ``cd_values`` array are used in two subtly different ways within HDF5.

The first use is in *passing* auxiliary data for a filter from the caller to the library when initially creating a dataset.
This happens *directly* in an ``H5Pset_filter()`` (`see here <https://docs.hdfgroup.org/hdf5/develop/group___o_c_p_l.html#ga191c567ee50b2063979cdef156a768c5>`_) call.

The second use is in *persisting* auxiliary data for a filter to the dataset's object *header* in a file.
This happens *indirectly* as part of an ``H5Dcreate()`` call.

When a dataset creation property list includes a filter, the filter's ``set_local()`` method is called (see `H5Zregister() <https://docs.hdfgroup.org/hdf5/develop/group___h5_z.html>`_) as part of the ``H5Dcreate`` call.
In the filter's ``set_local()`` method, the ``cd_values`` that were *passed* by the caller (in ``H5Pset_filter()``) are often modified (via ``H5Pmodify_filter()`` (`see here <https://docs.hdfgroup.org/hdf5/develop/group___o_c_p_l.html#title10>`__) before they are *persisted* to the dataset's object header in a file.

Among other things, this design allows a filter to be generally configured for *any* dataset in a file and then adjusted as necessary to handle such things as data type and/or dimensions when it is applied to a specific dataset.
Long story short, the data stored in ``cd_values`` of the dataset object's header in the file are often not the same values passed by the caller when the dataset was created.

To make matters a tad more complex, the ``cd_values`` data is treated by HDF5_ as an array of C typed, 4-byte, ``unsigned integer`` values.
Furthermore, regardless of `endianness <https://en.wikipedia.org/wiki/Endianness>`__ of the data producer, the persisted values are always stored in little-endian format in the dataset object header in the file.
Nonetheless, if the persisted ``cd_values`` data is ever retrieved (e.g. via ``H5Pget_filter_by_id()`` (`see here <https://docs.hdfgroup.org/hdf5/develop/group___o_c_p_l.html#title7>`__), the HDF5_ library ensures the data is returned to callers with proper endianness.
When command-line tools like ``h5ls`` and ``h5dump`` print ``cd_values``, the data will be displayed correctly.

Handling double precision auxiliary data via ``cd_values`` is still more complicated because a single double precision value will span multiple entries in ``cd_values`` in almost all cases.
Setting aside the possibility of differing floating point formats between the producer and consumers, any endianness handling the HDF5_ library does for the 4-byte entries in ``cd_values`` will certainly not ensure proper endianness handling of larger values.
It is impossible for command-line tools like ``h5ls`` and ``h5dump`` to display such data correctly.

Fortunately, the ZFP_ library has already been designed to handle these issues as part of the ZFP_'s *native* stream header.
But, the ZFP_ library handles these issues in an endian-agnostic way. 
Consequently, the H5Z-ZFP_ filter uses the ``cd_values`` that is persisted to a dataset's object header to store ZFP_'s stream header.
ZFP_'s stream header is stored starting at ``&cd_values[1]``. 
``cd_values[0]`` is used to stored H5Z-ZFP_ filter and ZFP_ library and ZFP_ encoder version information.

This also means that H5Z-ZFP_ avoids the overhead of duplicating the ZFP_ stream header in each dataset chunk.
For larger chunks, these savings are probably not too terribly significant.
