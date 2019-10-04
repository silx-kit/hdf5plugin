hdf5plugin
==========

This module provides HDF5 compression filters (namely: blosc, bitshuffle and lz4) and registers them to the HDF5 library used by `h5py <https://www.h5py.org>`_.

Supported platforms are: Linux, Windows, macOS.

Whenever possible, HDF5 compression filter plugins are best installed system-wide or through Anaconda (`blosc-hdf5-plugin <https://anaconda.org/conda-forge/blosc-hdf5-plugin>`_, `hdf5-lz4 <https://anaconda.org/nsls2forge/hdf5-lz4>`_).
Yet, `hdf5plugin` provides a generic way to enable `h5py` with the provided HDF5 compression filters.

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

To use it, just use ``import hdf5plugin`` and supported compression filters are available from `h5py <https://www.h5py.org>`_.

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

* Compression option helpers (See `Compression options`_):

  - ``hdf5plugin.BSHUF_LZ4_OPTS``: bitshuffle filter options for default block size and LZ4 compression.
  - ``hdf5plugin.blosc_options(level=5, shuffle='byte', compression='blosclz')``: Function to prepare ``compression_opts`` parameter to use with blosc compression.

* ``hdf5plugin.FILTERS``: A dictionary mapping provided filters to their ID
* ``hdf5plugin.PLUGINS_PATH``: The directory where the provided filters library are stored.

Compression options
*******************

Compression filters can be configured with the ``compression_opts`` argument of `h5py.Group.create_dataset <http://docs.h5py.org/en/stable/high/group.html#Group.create_dataset>`_ method by providing a tuple of integers.

The meaning of those integers is filter dependent and is described below.

bitshuffle
..........

compression_opts: (**block_size**, **lz4 compression**)

- **block size**: Number of elements (not bytes) per block.
  It MUST be a mulitple of 8.
  Default: 0 for a block size of about 8 kB.
- **lz4 compression**: 0: disabled (default), 2: enabled.

By default the filter uses bitshuffle, but do NOT compress with LZ4.

Example: Dataset compressed with bitshuffle+LZ4

.. code-block:: python

  f = h5py.File('test.h5', 'w')
  f.create_dataset('bitshuffle_with_lz4',
                   data=numpy.arange(100),
                   compression=hdf5plugin.BSHUF,
                   compression_opts=(0, 2))  # or hdf5plugin.BSHUF_LZ4_OPTS
  f.close()

blosc
.....

compression_opts: (0, 0, 0, 0, **compression level**, **shuffle**, **compression**)

- First 4 values are reserved.
- **compression level**:
  From 0 (no compression) to 9 (maximum compression).
  Default: 5.
- **shuffle**: Shuffle filter:

  * 0: no shuffle
  * 1: byte shuffle
  * 2: bit shuffle

- **compression**: The compressor blosc ID:

  * 0: blosclz (default)
  * 1: lz4
  * 2: lz4hc
  * 3: snappy (not available in `hdf5plugin`)
  * 4: zlib
  * 5: zstd

By default the filter uses byte shuffle and blosclz.

Example: Dataset compressed with bitshuffle+lz4

.. code-block:: python

  f = h5py.File('test.h5', 'w')
  f.create_dataset(
      'data',
      data=numpy.arange(100),
      compression=hdf5plugin.BLOSC,
      compression_opts=hdf5plugin.blosc_options(
          shuffle='bit', compression='lz4'))
      # or compression_opts=(0, 0, 0, 0, 9, 2, 1)
  f.close()

lz4
...

compression_opts: (**block_size**,)

- **block size**: Number of bytes per block.
  Default 0 for a block size of 1GB.
  It MUST be < 1.9 GB.

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

* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5-blosc/LICENSES/>`_ and `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/c-blosc/LICENSES/>`_
* lz4: See `src/LZ4/COPYING  <https://github.com/silx-kit/hdf5plugin/blob/master/src/LZ4/COPYING>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5/COPYING>`_.
