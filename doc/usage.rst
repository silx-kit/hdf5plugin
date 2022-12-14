=======
 Usage
=======

.. currentmodule:: hdf5plugin

``hdf5plugin`` allows using additional HDF5 compression filters with `h5py`_ for reading and writing compressed datasets.

Read compressed datasets
++++++++++++++++++++++++

In order to read compressed dataset with `h5py`_, use:

.. code-block:: python

    import hdf5plugin

It registers ``hdf5plugin`` supported compression filters with the HDF5 library used by `h5py`_.
Hence, HDF5 compressed datasets can be read as any other dataset (see `h5py documentation <https://docs.h5py.org/en/stable/high/dataset.html#reading-writing-data>`_).

Write compressed datasets
+++++++++++++++++++++++++

As for reading compressed datasets, ``import hdf5plugin`` is required to enable the supported compression filters.

To create a compressed dataset use `h5py.Group.create_dataset`_ and set the ``compression`` and ``compression_opts`` arguments.

``hdf5plugin`` provides helpers to prepare those compression options: `Bitshuffle`_, `Blosc`_, `BZip2`_, `FciDecomp`_, `LZ4`_, `SZ`_, `SZ3`_, `Zfp`_, `Zstd`_.

Sample code:

.. code-block:: python

    import numpy
    import h5py
    import hdf5plugin

    # Compression
    f = h5py.File('test.h5', 'w')
    f.create_dataset('data', data=numpy.arange(100), **hdf5plugin.LZ4())
    f.close()

    # Decompression
    f = h5py.File('test.h5', 'r')
    data = f['data'][()]
    f.close()

Relevant `h5py`_ documentation: `Filter pipeline <https://docs.h5py.org/en/stable/high/dataset.html#filter-pipeline>`_ and `Chunked Storage <https://docs.h5py.org/en/stable/high/dataset.html#chunked-storage>`_.


Bitshuffle
==========

.. autoclass:: Bitshuffle
   :members:
   :undoc-members:

Blosc
=====

.. autoclass:: Blosc
   :members:
   :undoc-members:

Blosc2
======

.. autoclass:: Blosc2
   :members:
   :undoc-members:

BZip2
=====

.. autoclass:: BZip2
   :members:
   :undoc-members:

FciDecomp
=========

.. autoclass:: FciDecomp
   :members:
   :undoc-members:

LZ4
===

.. autoclass:: LZ4
   :members:
   :undoc-members:

SZ
==

.. autoclass:: SZ
   :members:
   :undoc-members:

SZ3
===

.. autoclass:: SZ3
   :members:
   :undoc-members:

Zfp
===

.. autoclass:: Zfp
   :members:
   :undoc-members:

Zstd
====

.. autoclass:: Zstd
   :members:
   :undoc-members:

Get information about hdf5plugin
++++++++++++++++++++++++++++++++

Constants:

.. py:data:: PLUGIN_PATH

   Directory where the provided HDF5 filter plugins are stored.

Functions:

.. autofunction:: get_filters

.. autofunction:: get_config

Manage registered filters
+++++++++++++++++++++++++

When imported, `hdf5plugin` initialises and registers the filters it embeds if there is no already registered filters for the corresponding filter IDs.

`h5py`_ gives access to HDF5 functions handling registered filters in `h5py.h5z`_.
This module allows checking the filter availability and registering/unregistering filters.

`hdf5plugin` provides an extra `register` function to register the filters it provides, e.g., to override an already loaded filters.
Registering with this function is required to perform additional initialisation and enable writing compressed data with the given filter.

.. autofunction:: register

Use HDF5 filters in other applications
++++++++++++++++++++++++++++++++++++++

Non `h5py`_ or non-Python users can also benefit from the supplied HDF5 compression filters for reading compressed datasets by setting the ``HDF5_PLUGIN_PATH`` environment variable the value of ``hdf5plugin.PLUGIN_PATH``, which can be retrieved from the command line with::

    python -c "import hdf5plugin; print(hdf5plugin.PLUGIN_PATH)"

For instance::

    export HDF5_PLUGIN_PATH=$(python -c "import hdf5plugin; print(hdf5plugin.PLUGIN_PATH)")

should allow MatLab or IDL users to read data compressed using the supported plugins.

Setting the ``HDF5_PLUGIN_PATH`` environment variable allows already existing programs or Python code to read compressed data without any modification.

.. _h5py: https://www.h5py.org
.. _h5py.h5z: https://github.com/h5py/h5py/blob/master/h5py/h5z.pyx
.. _h5py.Group.create_dataset: https://docs.h5py.org/en/stable/high/group.html#h5py.Group.create_dataset
