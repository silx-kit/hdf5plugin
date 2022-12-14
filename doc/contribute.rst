============
 Contribute
============

This project follows the standard open-source project github workflow, which is described in other projects like `matplotlib <https://matplotlib.org/devel/contributing.html#contributing-code>`_ or `scikit-image <https://scikit-image.org/docs/dev/contribute.html>`_.

Testing
=======

To run self-contained tests, from Python:

.. code-block:: python

  import hdf5plugin.test
  hdf5plugin.test.run_tests()

Or, from the command line::

  python -m hdf5plugin.test

To also run tests relying on actual HDF5 files, run from the source directory::

  python test/test.py

This tests the installed version of `hdf5plugin`.

Building documentation
======================

Documentation relies on `Sphinx <https://www.sphinx-doc.org>`_.

To build documentation, run from the project root directory::

   python setup.py build
   PYTHONPATH=build/lib.<os>-<machine>-<pyver>/ sphinx-build -b html doc/ build/html

Guidelines to add a compression filter
======================================

This briefly describes the steps to add a HDF5 compression filter to the zoo.

* Add the source of the HDF5 filter and compression algorithm code in a subdirectory in ``src/[filter]``.
  Best is to use ``git subtree`` else copy the files there (including the license file).
  A released version of the filter + compression library should be used.

  ``git subtree`` command::

    git subtree add --prefix=src/[filter]  [git repository]  [release tag] --squash

* Update ``setup.py`` to build the filter dynamic library by adding an extension using the ``HDF5PluginExtension`` class (a subclass of ``setuptools.Extension``) which adds extra files and compile options to enable dynamic loading of the filter.
  The name of the extension should be ``hdf5plugin.plugins.libh5<filter_name>``.

* In case of import errors related to HDF5-related undefined symbols, add eventual missing functions under ``src/hdf5_dl.c``.

* Add a "CONSTANT" in ``src/hdf5plugin/_filters.py`` named with the ``FILTER_NAME_ID`` which value is the HDF5 filter ID
  (See `HDF5 registered filters <https://portal.hdfgroup.org/display/support/Registered+Filters>`_).

* Add a compression options helper class named ``FilterName`` in ``hdf5plugins/_filters.py`` which should inherit from ``_FilterRefClass``.
  This is intended to ease the usage of ``h5py.Group.create_dataset`` ``compression_opts`` argument.
  It must have a `filter_name` class attribute with the same name as in the extension defined in ``setup.py`` (without the ``libh5`` prefix) .
  This name is used to find the filter library.

* Add ``FilterName`` to ``hdf5plugin._filters.FILTER_CLASSES``.

* Add to ``hdf5plugin/__init__.py`` the import of the filter ID and helper class:
  ``from ._filters import FILTER_NAME_ID, FilterName  # noqa``

* Add tests:

  - In ``test/test.py`` for testing reading a compressed file that was produced with another software.
  - In ``src/hdf5plugin/test.py`` for tests that writes data using the compression filter and the compression options helper function and reads back the data.

* Update the ``doc/information.rst`` file to document:

  - The version of the HDF5 filter that is embedded in ``hdf5plugin``.
  - The license of the filter (by adding a link to the license file).

* Update the ``doc/usage.rst`` file to document:

  - The ``hdf5plugin.<FilterName>`` compression argument helper class.

* Update ``doc/contribute.rst`` to document the format of ``compression_opts`` expected by the filter
  (see `h5py custom compression filters <https://docs.h5py.org/en/stable/high/dataset.html#custom-compression-filters>`_).

Low-level compression filter arguments
======================================

Compression filters can be configured with the ``compression_opts`` argument of `h5py.Group.create_dataset <http://docs.h5py.org/en/stable/high/group.html#Group.create_dataset>`_ method by providing a tuple of integers.

The meaning of those integers is filter dependent and is described below.

bitshuffle
..........

compression_opts: (**block_size**, **compression**, **level**)

- **block size**: Number of elements (not bytes) per block.
  It MUST be a mulitple of 8.
  Default: 0 for a block size of about 8 kB.
- **compression**:

  * 0: No compression
  * 2: LZ4
  * 3: Zstd

- **level**: Compression level, only used with Zstd compression.

By default the filter uses bitshuffle, but does NOT compress with LZ4.

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
  * 3: snappy
  * 4: zlib
  * 5: zstd

By default the filter uses byte shuffle and blosclz.

blosc2
......

compression_opts: (0, 0, 0, 0, **compression level**, **filter**, **compression**)

- First 4 values are reserved.
- **compression level**:
  From 0 (no compression) to 9 (maximum compression).
  Default: 5.
- **filter**: Pre-compression filter:

  * 0: no shuffle
  * 1: byte shuffle
  * 2: bit shuffle
  * 3: delta: diff current block with first one
  * 4: truncate precision: Truncate mantissa for floating point types

- **compression**: The compressor blosc ID:

  * 0: blosclz (default)
  * 1: lz4
  * 2: lz4hc
  * 3: unused
  * 4: zlib
  * 5: zstd

By default the filter uses byte shuffle and blosclz.

bzip2
.....

compression_opts: (**block size**,)

- **block_size**: Size of the blocks as a multiple of 100k.
  It must be in the range [1, 9].

lz4
...

compression_opts: (**block_size**,)

- **block size**: Number of bytes per block.
  Default 0 for a block size of 1GB.
  It MUST be < 1.9 GB.

sz
..

compression_opts:

- **error_bound_mode** (int32)
- **abs_error high** (big endian float64)
- **abs_error low**
- **rel_error high** (big endian float64)
- **rel_error low**
- **pw_rel_error high** (big endian float64)
- **pw_rel_error low**
- **psnr high** (big endian float64)
- **psnr low**

The `set_local` function prepends:

- For **dim size** from 2 to 5:

  (**dim size**, **data type**, **r1**, **r2**, **r3** (if **dim size** >= 3), **r4** (if **dim size** >= 4), **r5** (if **dim size** == 5))

  **rX** are set up to **dim size** (e.g., For **dim size** == 2 only **r1** and **r2** are used)

- For **dim size** == 1: **r1** is stored on 64 bits:

  (**dim size**, **data type**, **r1 most-significant bytes**, **r1 least-significant bytes**)

sz3
...

compression_opts:

- **mode**
- **abs_error high** (big endian float64)
- **abs_error low**
- **rel_error high** (big endian float64)
- **rel_error low**
- **norm2 high** (big endian float64)
- **norm2 low**
- **psnr high** (big endian float64)
- **psnr low**

zfp
...

For more information, see `zfp modes <https://zfp.readthedocs.io/en/latest/modes.html>`_ and `hdf5-zfp generic interface <https://h5z-zfp.readthedocs.io/en/latest/interfaces.html#generic-interface>`_.

The first value of *compression_opts* is **mode**.
The following values depends on the value of **mode**:

- *Fixed-rate* mode:       (1, 0, **rateHigh**, **rateLow**, 0, 0)
  Rate, i.e., number of compressed bits per value, as a double stored as:

  - **rateHigh**: High 32-bit word of the rate double.
  - **rateLow**: Low 32-bit word of the rate double.

- *Fixed-precision* mode:  (2, 0, **prec**, 0, 0, 0)

  - **prec**: Number of uncompressed bits per value.

- *Fixed-accuracy* mode:   (3, 0, **accHigh**, **accLow**, 0, 0)
  Accuracy, i.e., absolute error tolerance, as a double stored as:

  - **accHigh**: High 32-bit word of the accuracy double.
  - **accLow**: Low 32-bit word of the accuracy double.

- *Expert* mode:     (4, 0, **minbits**, **maxbits**, **maxprec**, **minexp**)

  - **minbits**: Minimum number of compressed bits used to represent a block.
  - **maxbits**: Maximum number of bits used to represent a block.
  - **maxprec**: Maximum number of bit planes encoded.
  - **minexp**: Smallest absolute bit plane number encoded.

- *Reversible* mode: (5, 0, 0, 0, 0, 0)

zstd
....

compression_opts: (**clevel**,)

- **clevel**:
  Compression level from 1 (lowest compression) to 22 (maximum compression).
  Ultra compression extends from 20 through 22. Default: 3.
