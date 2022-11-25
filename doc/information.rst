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

* **LZ4 plugin** (commit `d48f960 <https://github.com/nexusformat/HDF5-External-Filter-Plugins/tree/d48f96064cb6e229ede4bf5e5c0e1935cf691036>`_) and **lz4** (v1.9.3): https://github.com/nexusformat/HDF5-External-Filter-Plugins and https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8/internal-complibs/lz4-1.9.3
* **bitshuffle plugin** (0.4.2 + patch `PR #122 <https://github.com/kiyo-masui/bitshuffle/pull/122>`_) and **zstd** (v1.5.2): https://github.com/kiyo-masui/bitshuffle and https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8/internal-complibs/zstd-1.5.2
* **bzip2 plugin** (from PyTables v3.7.0) and **bzip2** (v1.0.8): https://github.com/PyTables/PyTables/, https://sourceware.org/git/bzip2.git
* **hdf5-blosc plugin** (v1.0.0), **c-blosc** (commit `9dc93b1 <https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8>`_) and **snappy** (v1.1.9): https://github.com/Blosc/hdf5-blosc, https://github.com/Blosc/c-blosc and https://github.com/google/snappy
* **FCIDECOMP plugin** (v1.0.2) and **CharLS** (1.x branch, commit `25160a4 <https://github.com/team-charls/charls/tree/25160a42fb62e71e4b0ce081f5cb3f8bb73938b5>`_):
  ftp://ftp.eumetsat.int/pub/OPS/out/test-data/Test-data-for-External-Users/MTG_FCI_Test-Data/FCI_Decompression_Software_V1.0.2 and
  https://github.com/team-charls/charls
* **SZ plugin** (commit `f36afa4ae9 <https://github.com/szcompressor/SZ/commit/f36afa4ae9da0a8c50da3b9e89bfa89c8908b200>`_), `SZ <https://github.com/szcompressor/SZ>`_, `zlib <https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8/internal-complibs/zlib-1.2.11>`_ (v1.2.11) and `zstd <https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8/internal-complibs/zstd-1.5.2>`_ (v1.5.2)
* **HDF5-ZFP plugin** (commit `cd5422c <https://github.com/LLNL/H5Z-ZFP/tree/cd5422c146836e17c7a0380bfb05cf52d0c4467c>`_) and **zfp** (v1.0.0): https://github.com/LLNL/H5Z-ZFP and https://github.com/LLNL/zfp
* **HDF5Plugin-Zstandard** (commit `d5afdb5 <https://github.com/aparamon/HDF5Plugin-Zstandard/tree/d5afdb5f04116d5c2d1a869dc9c7c0c72832b143>`_) and **zstd** (v1.5.2): https://github.com/aparamon/HDF5Plugin-Zstandard and https://github.com/Blosc/c-blosc/tree/9dc93b1de7c1ff6265d0ae554bd79077840849d8/internal-complibs/zstd-1.5.2

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/LICENSE>`_.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5-blosc/LICENSES/>`_, `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc/LICENSES/>`_ and `src/snappy/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/snappy/COPYING>`_
* bzip2: See `src/PyTables/LICENSE.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/PyTables/LICENSE.txt>`_ and `src/bzip2/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/bzip2/LICENSE>`_
* lz4: See `src/LZ4/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/LZ4/COPYING>`_, `src/LZ4/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/LZ4/LICENSE>`_ and `src/c-blosc/LICENSES/LZ4.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc/LICENSES/LZ4.txt>`_
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/fcidecomp/LICENSE.txt>`_ and `src/charls/src/License.txt  <https://github.com/silx-kit/hdf5plugin/blob/main/src/charls/src/License.txt>`_
* SZ: See `src/SZ/copyright-and-BSD-license.txt <https://github.com/silx-kit/hdf5plugin/blob/main/src/SZ/copyright-and-BSD-license.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/zfp/LICENSE>`_
* zstd: See `src/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/HDF5Plugin-Zstandard/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5/COPYING>`_.
