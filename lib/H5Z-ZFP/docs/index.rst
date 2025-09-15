==================
Welcome to H5Z-ZFP
==================

H5Z-ZFP_ is a compression filter for HDF5_ using the ZFP_ compression library, supporting *lossy* and *lossless* compression of floating point and integer data to meet `bitrate <https://zfp.readthedocs.io/en/release0.5.4/modes.html#mode-fixed-rate>`_, `accuracy <https://zfp.readthedocs.io/en/release0.5.4/modes.html#mode-fixed-accuracy>`_, and/or `precision <https://zfp.readthedocs.io/en/release0.5.4/modes.html#fixed-precision-mode>`_ targets.
The filter uses the `registered <https://portal.hdfgroup.org/display/support/Filters#Filters-32013>`__ HDF5_ filter ID, ``32013``.
It supports single and double precision floating point and integer data *chunked* in 1, 2 or 3 dimensions.
The filter will function on datasets of more than 3 dimensions (or 4 dimensions for ZFP_ versions 0.5.4 and newer), albeit at the possible expense of compression performance, as long as no more than 3 (or 4) dimensions of the HDF5 dataset chunking are of size greater than 1.

Contents:

.. toctree::
   :maxdepth: 1
   :glob:

   installation
   interfaces
   hdf5_chunking
   cd_vals
   direct
   h5repack
   endian_issues
   tests
