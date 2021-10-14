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

* :doc:`Presentation <hdf5plugin_EuropeanHUG2021>` at the `European HDF Users Group (HUG) Summer 2021 <https://www.hdfgroup.org/hug/europeanhug21/>`_:

  - :doc:`Presentation material <hdf5plugin_EuropeanHUG2021>`
    (:download:`Notebook <hdf5plugin_EuropeanHUG2021.ipynb>`),
    `Video <https://youtu.be/DP-r2omEnrg>`_


HDF5 filters and compression libraries
--------------------------------------

HDF5 compression filters and compression libraries sources were obtained from:

* LZ4 plugin (v0.1.0) and lz4 (v1.3.0, tag r122): https://github.com/nexusformat/HDF5-External-Filter-Plugins, https://github.com/lz4/lz4
* bitshuffle plugin (0.3.5): https://github.com/kiyo-masui/bitshuffle
* hdf5-blosc plugin (v1.0.0), c-blosc (v1.21.1) and snappy (v1.1.8): https://github.com/Blosc/hdf5-blosc, https://github.com/Blosc/c-blosc and https://github.com/google/snappy
* FCIDECOMP plugin (v1.0.2) and CharLS (branch 1.x-master SHA1 ID: 25160a42fb62e71e4b0ce081f5cb3f8bb73938b5):
  ftp://ftp.eumetsat.int/pub/OPS/out/test-data/Test-data-for-External-Users/MTG_FCI_Test-Data/FCI_Decompression_Software_V1.0.2 and
  https://github.com/team-charls/charls
* HDF5-ZFP plugin (v1.0.1) and zfp (v0.5.5): https://github.com/LLNL/H5Z-ZFP and https://github.com/LLNL/zfp
* HDF5Plugin-Zstandard (commit d5afdb5) and zstd (v1.5.0): https://github.com/aparamon/HDF5Plugin-Zstandard and https://github.com/Blosc/c-blosc/tree/v1.21.1/internal-complibs/zstd-1.5.0

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/LICENSE>`_.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5-blosc/LICENSES/>`_, `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/main/src/c-blosc/LICENSES/>`_ and `src/snappy/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/snappy/COPYING>`_
* lz4: See `src/LZ4/COPYING  <https://github.com/silx-kit/hdf5plugin/blob/main/src/LZ4/COPYING>`_ and `src/lz4-r122/LICENSE  <https://github.com/silx-kit/hdf5plugin/blob/main/src/lz4-r122/LICENSE>`_
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/fcidecomp/LICENSE.txt>`_ and `src/charls/src/License.txt  <https://github.com/silx-kit/hdf5plugin/blob/main/src/charls/src/License.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/zfp/LICENSE>`_
* zstd: See `src/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/HDF5Plugin-Zstandard/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/main/src/hdf5/COPYING>`_.
