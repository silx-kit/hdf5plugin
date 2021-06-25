=======
 Usage
=======

.. currentmodule:: hdf5plugin

To use it, just use ``import hdf5plugin`` and supported compression filters are available from `h5py <https://www.h5py.org>`_.

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

``hdf5plugin`` provides:

* Compression option helper classes to prepare arguments to provide to ``h5py.Group.create_dataset``:

  - `Bitshuffle`_
  - `Blosc`_
  - `FciDecomp`_
  - `LZ4`_
  - `Zfp`_


* The HDF5 filter ID of embedded plugins:

  - ``BLOSC_ID``
  - ``BSHUF_ID``
  - ``FCIDECOMP_ID``
  - ``LZ4_ID``
  - ``ZFP_ID``
  - ``ZSTD_ID``

* ``FILTERS``: A dictionary mapping provided filters to their ID
* ``PLUGINS_PATH``: The directory where the provided filters library are stored.

Bitshuffle
==========

.. autoclass:: Bitshuffle
   :undoc-members: filter_id

Blosc
=====

.. autoclass:: Blosc
   :undoc-members: filter_id

FciDecomp
=========

.. autoclass:: FciDecomp
   :undoc-members: filter_id

LZ4
===

.. autoclass:: LZ4
   :undoc-members: filter_id

Zfp
===

.. autoclass:: Zfp
   :undoc-members: filter_id

Zstd
====

.. autoclass:: Zstd
   :undoc-members: filter_id
