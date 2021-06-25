hdf5plugin
==========

*hdf5plugin* provides HDF5 compression filters (namely: blosc, bitshuffle, lz4, FCIDECOMP, ZFP, zstd) and makes them usable from `h5py <https://www.h5py.org>`_.

See `documentation <http://www.silx.org/doc/hdf5plugin/latest/>`_.

Installation
------------

To install, run::

     pip install hdf5plugin [--user]
     
or, with conda (https://anaconda.org/conda-forge/hdf5plugin)::

    conda install -c conda-forge hdf5plugin

To install from source and recompile the HDF5 plugins, run::

     pip install hdf5plugin --no-binary hdf5plugin [--user]

Installing from source can achieve better performances by enabling AVX2 and OpenMP if available.

For more details, see the `installation documentation <http://www.silx.org/doc/hdf5plugin/latest/install.html>`_.

How-to use
----------

To use it, just use ``import hdf5plugin`` and supported compression filters are available from `h5py <https://www.h5py.org>`_.

For details, see `Usage documentation <http://www.silx.org/doc/hdf5plugin/latest/usage.html>`_.

License
-------

The source code of *hdf5plugin* itself is licensed under the `MIT license <LICENSE>`_.

Embedded HDF5 compression filters are licensed under different open-source licenses:
see the `license documentation <http://www.silx.org/doc/hdf5plugin/latest/information.html#license>`_.
