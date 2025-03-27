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

* `LZ4 plugin <https://github.com/nexusformat/HDF5-External-Filter-Plugins>`_ 
  (commit `49e3b65 <https://github.com/nexusformat/HDF5-External-Filter-Plugins/tree/49e3b65eca772bca77af13ba047d8b577673afba>`_)
  using LZ4.
* `bitshuffle plugin <https://github.com/kiyo-masui/bitshuffle>`_ (v0.5.2) using LZ4 and ZStd.
* bzip2 plugin (from `PyTables <https://github.com/PyTables/PyTables/>`_ v3.10.2)
  using `BZip2 <https://sourceware.org/git/bzip2.git>`_ (v1.0.8).
* `hdf5-blosc plugin <https://github.com/Blosc/hdf5-blosc>`_ (v1.0.1)
  using `c-blosc <https://github.com/Blosc/c-blosc>`_ (v1.21.6), LZ4, Snappy, ZLib and ZStd.
* hdf5-blosc2 plugin (from `PyTables <https://github.com/PyTables/PyTables/>`_ v3.10.2)
  using `c-blosc2 <https://github.com/Blosc/c-blosc2>`_ (v2.17.1), LZ4, ZLib and ZStd.
* `FCIDECOMP plugin <https://gitlab.eumetsat.int/open-source/data-tailor-plugins/fcidecomp>`_
  (`v2.1.1 <https://gitlab.eumetsat.int/open-source/data-tailor-plugins/fcidecomp/-/tree/2.1.1>`_)
  using `CharLS <https://github.com/team-charls/charls>`_ (v2.1.0).
* `SZ plugin <https://github.com/szcompressor/SZ>`_
  (commit `f466775 <https://github.com/szcompressor/SZ/tree/f4667759ead6a902110e80ff838ccdfddbc8dcd7>`_)
  using `SZ <https://github.com/szcompressor/SZ>`_, ZLib and ZStd.
* `H5Z-SPERR plugin <https://github.com/NCAR/H5Z-SPERR>`_ (v0.2.3) using `SPERR <https://github.com/NCAR/SPERR>`_ (v0.8.2).
* `SZ3 plugin <https://github.com/szcompressor/SZ3>`_
  (commit `4bbe9df7e4bcb <https://github.com/szcompressor/SZ3/commit/4bbe9df7e4bcb6ae6339fcb3033100da07fe7434>`_)
  using `SZ3 <https://github.com/szcompressor/SZ3>`_ and ZStd.
* `HDF5-ZFP plugin <https://github.com/LLNL/H5Z-ZFP>`_ (v1.1.1)
  using `zfp <https://github.com/LLNL/zfp>`_ (v1.0.1).
* `HDF5Plugin-Zstandard <https://github.com/aparamon/HDF5Plugin-Zstandard>`_
  (commit `d5afdb5 <https://github.com/aparamon/HDF5Plugin-Zstandard/tree/d5afdb5f04116d5c2d1a869dc9c7c0c72832b143>`_) using ZStd.

Sources of compression libraries shared accross multiple filters were obtained from:

* `LZ4 v1.10.0 <https://github.com/Blosc/c-blosc2/tree/v2.17.1/internal-complibs/lz4-1.10.0>`_
* `Snappy v1.2.1 <https://github.com/google/snappy>`_
* `ZStd v1.5.6 <https://github.com/Blosc/c-blosc2/tree/v2.17.1/internal-complibs/zstd-1.5.7>`_
* `ZLib v1.3.1 <https://github.com/Blosc/c-blosc/tree/v1.21.6/internal-complibs/zlib-1.3.1>`_

When compiled with Intel IPP, the LZ4 compression library is replaced with `LZ4 v1.9.3 <https://github.com/lz4/lz4/releases/tag/v1.9.3>`_ patched with a patch from Intel IPP 2021.7.0.

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/LICENSE>`_.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5-blosc/LICENSES/>`_, `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc/LICENSES/>`_ and `src/snappy/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/snappy/COPYING>`_
* blosc2: See `src/PyTables/LICENSE.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/PyTables/LICENSE.txt>`_  and `src/c-blosc2/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc2/LICENSES/>`_
* bzip2: See `src/PyTables/LICENSE.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/PyTables/LICENSE.txt>`_ and `src/bzip2/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/bzip2/LICENSE>`_
* lz4: See `src/LZ4/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/LZ4/COPYING>`_, `src/LZ4/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/LZ4/LICENSE>`_ and `src/c-blosc/LICENSES/LZ4.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc/LICENSES/LZ4.txt>`_
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/fcidecomp/LICENSE>`_ and `src/charls/LICENSE.md  <https://github.com/silx-kit/hdf5plugin/blob/main/src/charls/LICENSE.md>`_
* SPERR: See `src/H5Z-SPERR/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/H5Z-SPERR/LICENSE>`_ and `src/SPERR/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/SPERR/LICENSE>`_
* SZ: See `src/SZ/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/SZ/copyright-and-BSD-license.txt>`_
* SZ3: See `src/SZ3/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/SZ3/copyright-and-BSD-license.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/zfp/LICENSE>`_
* zstd: See `src/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/HDF5Plugin-Zstandard/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5/COPYING>`_.
