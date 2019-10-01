hdf5plugin
==========

This module provides HDF5 compression filters (namely: blosc, bitshuffle and lz4) and registers them to the HDF5 library used by `h5py <https://www.h5py.org>`_.

Supported platforms are: Linux, Windows, macOS.

The HDF5 plugin sources were obtained from:

* LZ4 plugin: https://github.com/nexusformat/HDF5-External-Filter-Plugins
* bitshuffle plugin: https://github.com/kiyo-masui/bitshuffle
* hdf5-blosc plugin and c-blosc: https://github.com/Blosc/hdf5-blosc and https://github.com/Blosc/c-blosc

Installation
------------

To install, just run::

     pip install hdf5plugin

To install locally, run::

     pip install hdf5plugin --user

Documentation
-------------

To use it, just use ''import hdf5plugin'' and supported compression plugins are available from `h5py <https://www.h5py.org>`_.

Sample code:

.. code-block:: python

  import numpy
  import h5py
  import hdf5plugin

  # Compression
  f = h5py.File('test.h5', 'w')
  f.create_dataset('data', data=numpy.arange(100), compression=hdf5plugin.LZ4)
  f.close()

  # Decompression
  f = h5py.File('test.h5', 'r')
  data = f['data'][()]
  f.close()

Dependencies
------------

* `h5py <https://www.h5py.org>`_

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license. Use it at your own risk.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.

Please read the different licenses:

* hdf5plugin itself: See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/LICENSE>`_
* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5-blosc/LICENSES/>`_ and `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/c-blosc/LICENSES/>`_
* lz4: See `src/HDF5-External-Filter-Plugins/LZ4/COPYING  <https://github.com/silx-kit/hdf5plugin/blob/master/src/HDF5-External-Filter-Plugins/LZ4/COPYING>`_
