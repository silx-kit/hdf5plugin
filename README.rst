hdf5plugin
==========

This module provides HDF5 compression filters (namely: blosc, bitshuffle and lz4) and registers them to the HDF5 library used by `h5py <https://www.h5py.org>`_.

Supported platforms are: Linux, Windows, macOS.

The HDF5 plugin sources were obtained from:

* LZ4 plugin (v0.1.0): https://github.com/nexusformat/HDF5-External-Filter-Plugins
* bitshuffle plugin (0.3.5): https://github.com/kiyo-masui/bitshuffle
* hdf5-blosc plugin (v1.0.0) and c-blosc (v1.17.0): https://github.com/Blosc/hdf5-blosc and https://github.com/Blosc/c-blosc

Installation
------------

To install, just run::

     pip install hdf5plugin

To install locally, run::

     pip install hdf5plugin --user

Documentation
-------------

To use it, just use ``import hdf5plugin`` and supported compression plugins are available from `h5py <https://www.h5py.org>`_.

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

``hdf5plugin`` provides:

* The HDF5 filter ID of embedded plugins:

  - ``hdf5plugin.BLOSC``
  - ``hdf5plugin.BSHUF``
  - ``hdf5plugin.LZ4``

* ``hdf5plugin.FILTERS``: A dictionary mapping provided filters to their ID
* ``hdf5plugin.PLUGINS_PATH``: The directory where the provided filters library are stored.


Dependencies
------------

* `h5py <https://www.h5py.org>`_


Testing
-------

To run self-contained tests, from Python:

.. code-block:: python

  import hdf5plugin.test
  hdf5plugin.test.run_tests()

Or, from the command line::

  python -m hdf5plugin.test

To also run tests relying on actual HDF5 files, run from the source directory::

  python test/test.py

This tests the installed version of `hdf5plugin`.

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/LICENSE>`_

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* hdf5plugin itself: * bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5-blosc/LICENSES/>`_ and `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/c-blosc/LICENSES/>`_
* lz4: See `src/LZ4/COPYING  <https://github.com/silx-kit/hdf5plugin/blob/master/src/LZ4/COPYING>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5/COPYING>`_.
