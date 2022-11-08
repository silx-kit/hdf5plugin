=========================
Direct Writes (and Reads)
=========================
The purpose of direct_ writes
is to enable an application to write data that is already compressed in memory *directly*
to an HDF5 file without first uncompressing it so the filter can then turn around and
compress it during write. However, once data is written to the file with a *direct* write,
consumers must still be able to read it without concern for how the producer wrote it.

Doing this requires the use of an advanced HDF5 function for direct_ writes.

At present, we demonstrate only minimal functionality here using *single chunking*, where
the chunk size is chosen to match the size of the entire dataset. To see an example of
code that does this, have a look at...

.. literalinclude:: ../test/test_write.c
   :language: c
   :linenos:
   :start-after: ZFP Array Example
   :end-before:  End of ZFP Array Example

In particular, look for the line using ``H5Dchunk_write`` in place of ``H5Dwrite``. In all
other respects, the code looks the same.

The test case for this code writes uncompressed data as a dataset named ``zfparr_original``,
the compressed dataset named ``zfparr_compressed`` using the filter and then the compressed
data a second time named ``zfparr_direct`` using a direct write. Then, the ``h5diff`` tool
is used to compare the data in the original and direct datasets.

Note that in order for consumers to work as normal, the producer must set dataset *creation*
properties as it ordinarily would using the H5Z-ZFP_ filter. In the call to ``H5Dchunk_write``,
the caller indicates to the HDF5 library not to invoke the filter via the ``filters`` mask
argument.

.. _direct: https://portal.hdfgroup.org/display/HDF5/H5D_WRITE_CHUNK
