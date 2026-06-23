=====================
 Project information
=====================

Releases
--------

Source code and pre-built binaries (aka Python wheels) for Windows, MacOS and
ManyLinux are available at the following places:

- `Wheels and source code on PyPi <https://pypi.org/project/hdf5plugin/>`_
- `Packages on conda-forge <https://anaconda.org/conda-forge/hdf5plugin>`_

For the history of modifications, see the :doc:`changelog`.

Project resources
-----------------

- `Source repository <https://github.com/silx-kit/hdf5plugin>`_
- `Issue tracker <https://github.com/silx-kit/hdf5plugin/issues>`_

`hdf5plugin` can be cited with its DOI: `10.5281/zenodo.7257761 <https://doi.org/10.5281/zenodo.7257761>`_.

Presentations
-------------

.. toctree::
   :hidden:

   hdf5plugin_EuropeanHUG2023/benchmark.ipynb
   hdf5plugin_EuropeanHUG2023/hdf5_compressed_chunk_direct_read.ipynb
   hdf5plugin_EuropeanHUG2023/presentation.ipynb


* `Presentation <https://indico.desy.de/event/39343/contributions/151492/>`_ at the `European HDF Users Group (HUG) Meeting 2023 <https://indico.desy.de/event/39343/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2023/presentation>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2023/presentation.ipynb>`),
    `Video EuHUG2023 <https://youtu.be/IyS_NgAwXuU>`_
  - Benchmark: :download:`script <hdf5plugin_EuropeanHUG2023/benchmark.py>`, :download:`display notebook <hdf5plugin_EuropeanHUG2023/benchmark.ipynb>`
  - :doc:`HDF5 compressed chunk direct read example <hdf5plugin_EuropeanHUG2023/hdf5_compressed_chunk_direct_read>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2023/hdf5_compressed_chunk_direct_read.ipynb>`)


* :doc:`Presentation <hdf5plugin_EuropeanHUG2022>` at the `European HDF Users Group (HUG) Meeting 2022 <https://www.hdfgroup.org/hug/europeanhug22/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2022>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2022.ipynb>`),
    `Video EuHUG2022 <https://youtu.be/Titp1XRBh9k>`_


* :doc:`Presentation <hdf5plugin_EuropeanHUG2021>` at the `European HDF Users Group (HUG) Summer 2021 <https://www.hdfgroup.org/hug/europeanhug21/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2021>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2021.ipynb>`),
    `Video EuHUG2021 <https://youtu.be/DP-r2omEnrg>`_


HDF5 filters and compression libraries
--------------------------------------

HDF5 compression filters and compression libraries sources were obtained from:

* `LZ4 plugin <https://github.com/HDFGroup/hdf5_plugins/tree/master/LZ4>`_
  (commit `5573db8 <https://github.com/HDFGroup/hdf5_plugins/tree/5573db8f9706e1df262752a9247e90a3d4bd57bd/LZ4>`_)
  using LZ4.
* `bitshuffle plugin <https://github.com/kiyo-masui/bitshuffle>`_ (v0.5.2) using LZ4 and ZStd.
* bzip2 plugin (from `PyTables <https://github.com/PyTables/PyTables/>`_ v3.10.2)
  using `BZip2 <https://sourceware.org/git/bzip2.git>`_ (v1.0.8).
* `hdf5-blosc plugin <https://github.com/Blosc/hdf5-blosc>`_ (v1.0.1)
  using `c-blosc <https://github.com/Blosc/c-blosc>`_ (v1.21.6), LZ4, Snappy, ZLib and ZStd.
* `hdf5-blosc2 plugin <https://github.com/Blosc/HDF5-Blosc2>`_
  (commit `e4d0f58 <https://github.com/Blosc/HDF5-Blosc2/tree/e4d0f583f39bf1d3e482aa4695b7dc95afb2b9b2>`_)
  using `c-blosc2 <https://github.com/Blosc/c-blosc2>`_ (v3.1.4), LZ4, ZFP, ZLib and ZStd.
* `FCIDECOMP plugin <https://gitlab.eumetsat.int/open-source/data-tailor-plugins/fcidecomp>`_
  (`v2.1.1 <https://gitlab.eumetsat.int/open-source/data-tailor-plugins/fcidecomp/-/tree/2.1.1>`_)
  using `CharLS <https://github.com/team-charls/charls>`_ (v2.1.0).
* `SZ plugin <https://github.com/szcompressor/SZ2>`_
  (commit `308bd06 <https://github.com/szcompressor/SZ2/tree/308bd06f0040ec0d5c22fb3fcb0428c306ba4df1>`_)
  using `SZ <https://github.com/szcompressor/SZ2>`_, ZLib and ZStd.
* `H5Z-SPERR plugin <https://github.com/NCAR/H5Z-SPERR>`_ (v0.2.3) using `SPERR <https://github.com/NCAR/SPERR>`_ (v0.8.5).
* `SZ3 plugin <https://github.com/szcompressor/SZ3>`_
  (commit `4bbe9df7e4bcb <https://github.com/szcompressor/SZ3/commit/4bbe9df7e4bcb6ae6339fcb3033100da07fe7434>`_)
  using `SZ3 <https://github.com/szcompressor/SZ3>`_ and ZStd.
* `HDF5-ZFP plugin <https://github.com/LLNL/H5Z-ZFP>`_ (v1.1.1) using ZFP.
* `ZStd plugin <https://github.com/HDFGroup/hdf5_plugins/tree/master/ZSTD>`_
  (commit `5573db8 <https://github.com/HDFGroup/hdf5_plugins/tree/5573db8f9706e1df262752a9247e90a3d4bd57bd/ZSTD>`_) using ZStd.

Sources of compression libraries shared accross multiple filters were obtained from:


* `LZ4 v1.10.0 <https://github.com/lz4/lz4>`_
* `Snappy v1.2.2 <https://github.com/google/snappy>`_
* `ZFP v1.0.1 <https://github.com/LLNL/zfp>`_
* `ZStd v1.5.7 <https://github.com/facebook/zstd>`_
* `ZLib v1.3.1 <https://github.com/Blosc/c-blosc/tree/v1.21.6/internal-complibs/zlib-1.3.1>`_

When compiled with Intel IPP, the LZ4 compression library is replaced with `LZ4 v1.9.3 <https://github.com/lz4/lz4/releases/tag/v1.9.3>`_ patched with a patch from Intel IPP 2021.7.0.

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/LICENSE>`_.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* HDF5 compression filters:

  * bitshuffle: `lib/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/bitshuffle/LICENSE>`_
  * blosc: `lib/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/lib/hdf5-blosc/LICENSES/>`_, `lib/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/lib/c-blosc/LICENSES/>`_
  * blosc2: `lib/PyTables/LICENSE.txt <https://github.com/silx-kit/hdf5plugin/blob/main/lib/PyTables/LICENSE.txt>`_  and `lib/c-blosc2/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/lib/c-blosc2/LICENSES/>`_
  * bzip2: `lib/PyTables/LICENSE.txt <https://github.com/silx-kit/hdf5plugin/blob/main/lib/PyTables/LICENSE.txt>`_ and `lib/bzip2/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/bzip2/LICENSE>`_
  * lz4: `lib/LZ4/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/lib/LZ4/COPYING>`_, `lib/LZ4/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/LZ4/LICENSE>`_
  * FCIDECOMP: `lib/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/fcidecomp/LICENSE>`_ and `lib/charls/LICENSE.md  <https://github.com/silx-kit/hdf5plugin/blob/main/lib/charls/LICENSE.md>`_
  * SPERR: `lib/H5Z-SPERR/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/H5Z-SPERR/LICENSE>`_ and `lib/SPERR/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/SPERR/LICENSE>`_
  * SZ: `lib/SZ/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/lib/SZ/copyright-and-BSD-license.txt>`_
  * SZ3: `lib/SZ3/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/lib/SZ3/copyright-and-BSD-license.txt>`_
  * zfp: `lib/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/H5Z-ZFP/LICENSE>`_
  * zstd: `lib/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/HDF5Plugin-Zstandard/LICENSE>`_

* Shared compression libraries:

  * LZ4: `lib/lz4-clib/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/lz4-clib/LICENSE>`_
  * Snappy: `lib/snappy/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/lib/snappy/COPYING>`_
  * ZFP: `lib/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/zfp/LICENSE>`_
  * ZLib: `lib/c-blosc/internal-complibs/zlib-1.3.1/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/c-blosc/internal-complibs/zlib-1.3.1/LICENSE>`_
  * ZStd: `lib/zstd/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/lib/zstd/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `lib/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/lib/hdf5/COPYING>`_.
