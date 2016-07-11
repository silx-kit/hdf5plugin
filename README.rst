hdf5plugin
==========

This module provides HDF5 plugins and sets the HDF5_PLUGIN_PATH to the appropriate value according to the version of python used.

The module is intended for use under windows because at this point no other shared libraries are provided.

The LZ4 plugin (H5Zlz4.dll) sources were obtained from https://github.com/nexusformat/HDF5-External-Filter-Plugins

The bitshuffle plugin (H5bshuf.dll) sources were obtained from https://github.com/kiyo-masui/bitshuffle


Installation
------------

To install, just run::

     pip install hdf5plugin

To install locally, run::

     pip install hdf5plugin --user

Dependencies
------------

No additional dependencies.

Documentation
-------------

To use it, just ''import hdf5plugin'' **before** importing h5py or PyTables.

License
-------

This code is licensed under the MIT license. Use it at your own risk.

The source code of the libraries is licensed under MIT (bitshuffle) and BSD like licenses (LZ4).

Please read the LICENSE_H5bshuf and LICENSE_H5Zlz4 respectively for details.

