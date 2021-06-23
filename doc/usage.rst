=======
 Usage
=======

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

  - `Bitshuffle(nelems=0, lz4=True)`_
  - `Blosc(cname='lz4', clevel=5, shuffle=SHUFFLE)`_
  - `FciDecomp()`_
  - `LZ4(nbytes=0)`_
  - `Zfp()`_


* The HDF5 filter ID of embedded plugins:

  - ``BLOSC_ID``
  - ``BSHUF_ID``
  - ``FCIDECOMP_ID``
  - ``LZ4_ID``
  - ``ZFP_ID``
  - ``ZSTD_ID``

* ``FILTERS``: A dictionary mapping provided filters to their ID
* ``PLUGINS_PATH``: The directory where the provided filters library are stored.

Bitshuffle(nelems=0, lz4=True)
==============================

This class takes the following arguments and returns the compression options to feed into ``h5py.Group.create_dataset`` for using the bitshuffle filter:

* **nelems** the number of elements per block, needs to be divisible by eight (default is 0, about 8kB per block)
* **lz4** if True the elements get compressed using lz4 (default is True)

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('bitshuffle_with_lz4', data=numpy.arange(100),
	      **hdf5plugin.Bitshuffle(nelems=0, lz4=True))
        f.close()

Blosc(cname='lz4', clevel=5, shuffle=SHUFFLE)
=============================================

This class takes the following arguments and returns the compression options to feed into ``h5py.Group.create_dataset`` for using the blosc filter:

* **cname** the compression algorithm, one of:

  * 'blosclz'
  * 'lz4' (default)
  * 'lz4hc'
  * 'snappy' (optional, requires C++11)
  * 'zlib'
  * 'zstd'

* **clevel** the compression level, from 0 to 9 (default is 5)
* **shuffle** the shuffling mode, in:

  * `Blosc.NOSHUFFLE` (0): No shuffle
  * `Blosc.SHUFFLE` (1): byte-wise shuffle (default)
  * `Blosc.BITSHUFFLE` (2): bit-wise shuffle

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('blosc_byte_shuffle_blosclz', data=numpy.arange(100),
            **hdf5plugin.Blosc(cname='blosclz', clevel=9, shuffle=hdf5plugin.Blosc.SHUFFLE))
        f.close()

FciDecomp()
===========

This class returns the compression options to feed into ``h5py.Group.create_dataset`` for using the FciDecomp filter:

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('fcidecomp', data=numpy.arange(100),
            **hdf5plugin.FciDecomp())
        f.close()

LZ4(nbytes=0)
=============

This class takes the number of bytes per block as argument and returns the compression options to feed into ``h5py.Group.create_dataset`` for using the lz4 filter:

* **nbytes** number of bytes per block needs to be in the range of 0 < nbytes < 2113929216 (1,9GB).
  The default value is 0 (for 1GB).

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('lz4', data=numpy.arange(100),
            **hdf5plugin.LZ4(nbytes=0))
        f.close()

Zfp()
=====

This class returns the compression options to feed into ``h5py.Group.create_dataset`` for using the zfp filter:

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('zfp', data=numpy.random.random(100),
            **hdf5plugin.Zfp())
        f.close()

The zfp filter compression mode is defined by the provided arguments.
The following compression modes are supported:

- **Fixed-rate** mode:
  For details, see `zfp fixed-rate mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-rate-mode>`_.

  .. code-block:: python

        f.create_dataset('zfp_fixed_rate', data=numpy.random.random(100),
            **hdf5plugin.Zfp(rate=10.0))

- **Fixed-precision** mode:
  For details, see `zfp fixed-precision mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-precision-mode>`_.

  .. code-block:: python

        f.create_dataset('zfp_fixed_precision', data=numpy.random.random(100),
            **hdf5plugin.Zfp(precision=10))

- **Fixed-accuracy** mode:
  For details, see `zfp fixed-accuracy mode <https://zfp.readthedocs.io/en/latest/modes.html#fixed-accuracy-mode>`_.

  .. code-block:: python

        f.create_dataset('zfp_fixed_accuracy', data=numpy.random.random(100),
            **hdf5plugin.Zfp(accuracy=0.001))

- **Reversible** (i.e., lossless) mode:
  For details, see `zfp reversible mode <https://zfp.readthedocs.io/en/latest/modes.html#reversible-mode>`_.

  .. code-block:: python

        f.create_dataset('zfp_reversible', data=numpy.random.random(100),
            **hdf5plugin.Zfp(reversible=True))

- **Expert** mode:
  For details, see `zfp expert mode <https://zfp.readthedocs.io/en/latest/modes.html#expert-mode>`_.

  .. code-block:: python

        f.create_dataset('zfp_expert', data=numpy.random.random(100),
            **hdf5plugin.Zfp(minbits=1, maxbits=16657, maxprec=64, minexp=-1074))

Zstd()
======

This class returns the compression options to feed into ``h5py.Group.create_dataset`` for using the Zstd filter:

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('zstd', data=numpy.arange(100),
            **hdf5plugin.Zstd())
        f.close()
