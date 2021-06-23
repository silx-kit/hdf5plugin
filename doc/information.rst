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

License
-------

The source code of *hdf5plugin* itself is licensed under the MIT license.
Use it at your own risk.
See `LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/LICENSE>`_.

The source code of the embedded HDF5 filter plugin libraries is licensed under different open-source licenses.
Please read the different licenses:

* bitshuffle: See `src/bitshuffle/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/bitshuffle/LICENSE>`_
* blosc: See `src/hdf5-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5-blosc/LICENSES/>`_ and `src/c-blosc/LICENSES/ <https://github.com/silx-kit/hdf5plugin/blob/master/src/c-blosc/LICENSES/>`_
* lz4: See `src/LZ4/COPYING  <https://github.com/silx-kit/hdf5plugin/blob/master/src/LZ4/COPYING>`_ and `src/lz4-r122/LICENSE  <https://github.com/silx-kit/hdf5plugin/blob/master/src/lz4-r122/LICENSE>`_
* FCIDECOMP: See `src/fcidecomp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/fcidecomp/LICENSE.txt>`_ and `src/charls/src/License.txt  <https://github.com/silx-kit/hdf5plugin/blob/master/src/charls/License.txt>`_
* zfp: See `src/H5Z-ZFP/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/H5Z-ZFP/LICENSE>`_ and `src/zfp/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/master/src/zfp/LICENSE>`_
* zstd: See `src/HDF5Plugin-Zstandard/LICENSE <https://github.com/silx-kit/hdf5plugin/blob/main/src/HDF5Plugin-Zstandard/LICENSE>`_

The HDF5 v1.10.5 headers (and Windows .lib file) used to build the filters are stored for convenience in the repository. The license is available here: `src/hdf5/COPYING <https://github.com/silx-kit/hdf5plugin/blob/master/src/hdf5/COPYING>`_.
