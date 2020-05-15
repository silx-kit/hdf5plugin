hdf5plugin
==========

This module provides HDF5 compression filters (namely: blosc, bitshuffle, lz4, FCIDECOMP, ZFP) and registers them to the HDF5 library used by `h5py <https://www.h5py.org>`_.

* Supported operating systems: Linux, Windows, macOS.
* Supported versions of Python: 2.7 and >= 3.4

`hdf5plugin` provides a generic way to enable the use of the provided HDF5 compression filters with `h5py` that can be installed via `pip` or `conda`.

Alternatives to install HDF5 compression filters are: system-wide installation on Linux or other conda packages: `blosc-hdf5-plugin <https://anaconda.org/conda-forge/blosc-hdf5-plugin>`_, `hdf5-lz4 <https://anaconda.org/nsls2forge/hdf5-lz4>`_.

The HDF5 plugin sources were obtained from:

* LZ4 plugin (v0.1.0): https://github.com/nexusformat/HDF5-External-Filter-Plugins
* bitshuffle plugin (0.3.5): https://github.com/kiyo-masui/bitshuffle
* hdf5-blosc plugin (v1.0.0) and c-blosc (v1.17.0): https://github.com/Blosc/hdf5-blosc and https://github.com/Blosc/c-blosc
* FCIDECOMP plugin (v1.0.2) and CharLS (branch 1.x-master SHA1 ID:25160a42fb62e71e4b0ce081f5cb3f8bb73938b5): ftp://ftp.eumetsat.int/pub/OPS/out/test-data/Test-data-for-External-Users/MTG_FCI_Test-Data/FCI_Decompression_Software_V1.0.2/ and https://github.com/team-charls/charls.git 
* HDF5-ZFP plugin (v1.0.1) and zfp (v0.5.5): https://github.com/LLNL/H5Z-ZFP and https://github.com/LLNL/zfp

Installation
------------

To install, run::

     pip install hdf5plugin [--user]
     
or, with conda (https://anaconda.org/conda-forge/hdf5plugin)::

    conda install -c conda-forge hdf5plugin

To install from source and recompile the HDF5 plugins, run::

     pip install hdf5plugin --no-binary hdf5plugin [--user]

Installing from source can achieve better performances by enabling AVX2 and OpenMP if available.

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

* ``FILTERS``: A dictionary mapping provided filters to their ID
* ``PLUGINS_PATH``: The directory where the provided filters library are stored.

Bitshuffle(nelems=0, lz4=True)
******************************

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
*********************************************

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
***********

This class returns the compression options to feed into ``h5py.Group.create_dataset`` for using the FciDecomp filter:

It can be passed as keyword arguments.

Sample code:

.. code-block:: python

        f = h5py.File('test.h5', 'w')
        f.create_dataset('fcidecomp', data=numpy.arange(100),
            **hdf5plugin.FciDecomp())
        f.close()

LZ4(nbytes=0)
*************

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
*****

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
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/fcidecomp/LICENSE.txt>`_ and `src/charls/src/License.txt  <https://github.com/silx-kit/hdf5plugin/blob/master/src/charls/License.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/zfp/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5/COPYING>`_.
