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
- Continuous integration: *hdf5plugin* is continuously tested on all three major
  operating systems:

  - Linux, MacOS, Windows: `GitHub Actions <https://github.com/silx-kit/hdf5plugin/actions>`_
  - Windows: `AppVeyor <https://ci.appveyor.com/project/ESRF/hdf5plugin>`_
- `Weekly builds <https://silx.gitlab-pages.esrf.fr/bob/hdf5plugin/>`_

`hdf5plugin` can be cited with its `Zenodo DOI <https://doi.org/10.5281/zenodo.7257761>`_.

Presentations
-------------

* :doc:`Presentation <hdf5plugin_EuropeanHUG2022>` at the `European HDF Users Group (HUG) Meeting 2022 <https://www.hdfgroup.org/hug/europeanhug22/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2022>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2022.ipynb>`),
    `Video <https://youtu.be/Titp1XRBh9k>`_


* :doc:`Presentation <hdf5plugin_EuropeanHUG2021>` at the `European HDF Users Group (HUG) Summer 2021 <https://www.hdfgroup.org/hug/europeanhug21/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2021>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2021.ipynb>`),
    `Video <https://youtu.be/DP-r2omEnrg>`_


HDF5 filters and compression libraries
--------------------------------------

HDF5 compression filters and compression libraries sources were obtained from:

* `LZ4 plugin <https://github.com/nexusformat/HDF5-External-Filter-Plugins>`_ 
  (commit `d48f960 <https://github.com/nexusformat/HDF5-External-Filter-Plugins/tree/d48f96064cb6e229ede4bf5e5c0e1935cf691036>`_)
  using LZ4.
* `bitshuffle plugin <https://github.com/kiyo-masui/bitshuffle>`_ (v0.5.1) using LZ4 and ZStd.
* bzip2 plugin (from `PyTables <https://github.com/PyTables/PyTables/>`_ v3.8.0)
  using `BZip2 <https://sourceware.org/git/bzip2.git>`_ (v1.0.8).
* `hdf5-blosc plugin <https://github.com/Blosc/hdf5-blosc>`_ (v1.0.0)
  using `c-blosc <https://github.com/Blosc/c-blosc>`_ (v1.21.4), LZ4, Snappy, ZLib and ZStd.
* hdf5-blosc2 plugin (from `PyTables <https://github.com/PyTables/PyTables/>`_ v3.8.0)
  using `c-blosc2 <https://github.com/Blosc/c-blosc2>`_ (v2.9.2), LZ4, ZLib and ZStd.
* `FCIDECOMP plugin <ftp://ftp.eumetsat.int/pub/OPS/out/test-data/Test-data-for-External-Users/MTG_FCI_Test-Data/FCI_Decompression_Software_V1.0.2>`_ (v1.0.2)
  using `CharLS <https://github.com/team-charls/charls>`_
  (1.x branch, commit `25160a4 <https://github.com/team-charls/charls/tree/25160a42fb62e71e4b0ce081f5cb3f8bb73938b5>`_).
* `SZ plugin <https://github.com/szcompressor/SZ>`_
  (commit `c25805c12b3 <https://github.com/szcompressor/SZ/commit/c25805c12b339d2cb2f406f95293b9a7313c4fb1>`_)
  using `SZ <https://github.com/szcompressor/SZ>`_, ZLib and ZStd.
* `SZ3 plugin <https://github.com/szcompressor/SZ3>`_
  (commit `4bbe9df7e4bcb <https://github.com/szcompressor/SZ3/commit/4bbe9df7e4bcb6ae6339fcb3033100da07fe7434>`_)
  using `SZ3 <https://github.com/szcompressor/SZ3>`_ and ZStd.
* `HDF5-ZFP plugin <https://github.com/LLNL/H5Z-ZFP>`_
  (commit `cd5422c <https://github.com/LLNL/H5Z-ZFP/tree/cd5422c146836e17c7a0380bfb05cf52d0c4467c>`_)
  using `zfp <https://github.com/LLNL/zfp>`_ (v1.0.0).
* `HDF5Plugin-Zstandard <https://github.com/aparamon/HDF5Plugin-Zstandard>`_
  (commit `d5afdb5 <https://github.com/aparamon/HDF5Plugin-Zstandard/tree/d5afdb5f04116d5c2d1a869dc9c7c0c72832b143>`_) using ZStd.

Sources of compression libraries shared accross multiple filters were obtained from:

* `LZ4 v1.9.4 <https://github.com/Blosc/c-blosc2/tree/v2.9.2/internal-complibs/lz4-1.9.4>`_
* `Snappy v1.1.10 <https://github.com/google/snappy>`_
* `ZStd v1.5.5 <https://github.com/Blosc/c-blosc2/tree/v2.9.2/internal-complibs/zstd-1.5.5>`_
* `ZLib v1.2.13 <https://github.com/Blosc/c-blosc/tree/v1.21.4/internal-complibs/zlib-1.2.13>`_

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
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/fcidecomp/LICENSE.txt>`_ and `src/charls/src/License.txt  <https://github.com/silx-kit/hdf5plugin/blob/main/src/charls/src/License.txt>`_
* SZ: See `src/SZ/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/SZ/copyright-and-BSD-license.txt>`_
* SZ3: See `src/SZ3/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/SZ3/copyright-and-BSD-license.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/zfp/LICENSE>`_
* zstd: See `src/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/HDF5Plugin-Zstandard/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5/COPYING>`_.
