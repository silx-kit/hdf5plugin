==================
Welcome to H5Z-ZFP
==================

H5Z-ZFP_ is a compression filter for HDF5_ using the
`ZFP compression library <http://computation.llnl.gov/projects/floating-point-compression>`_,
supporting *lossy* and *lossless* compression of floating point and integer data to meet
bitrate, accuracy, and/or precision targets. The filter uses the
`registered <https://support.hdfgroup.org/services/filters.html#zfp>`_ HDF5_ filter ID, ``32013``.
It supports single and double precision floating point and integer data *chunked* in 1, 2 or
3 dimensions.  The filter will function on datasets of more than 3 dimensions (or 4
dimensions for ZFP versions 0.5.4 and newer), albeit at the
possible expense of compression performance, as long as no more than 3
(or 4) dimensions of the HDF5 dataset chunking are of size greater than 1.

Contents:

.. toctree::
   :maxdepth: 1
   :glob:

   installation
   interfaces
   hdf5_chunking
   h5repack
   endian_issues
   tests
