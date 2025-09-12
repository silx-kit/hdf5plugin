.. _endian-issues:

=============
Endian Issues
=============

This section describes some issues related to `endianness <https://en.wikipedia.org/wiki/Endianness>`__ of producers and consumers of the data processed by H5Z-ZFP_.
This is likely less of an issue than it once was because almost all modern CPUs are `little-endian <https://www.reddit.com/r/linux/comments/3467gq/bigendian_is_effectively_dead/>`__.

That being said, the ZFP_ library writes an endian-independent stream.

There is an unavoidable inefficiency when reading ZFP_ compressed data on a machine with a different endianness than the writer (e.g. a *mixed* endian context). Upon reading data from storage and decompressing the read stream with ZFP_, the correct endianness is returned in the result from ZFP_ before the buffer is handed back to HDF5_ from the decompression filter.
This happens regardless of reader and writer endianness incompatibility.
However, the HDF5_ library expects to get from H5Z-ZFP_ the endianness of the data as it was stored to the file on the writer machine and expects to have to byte-swap that buffer before returning to it an endian-incompatible caller.

This means that in the H5Z-ZFP_ plugin, we wind up having to un-byte-swap an already correct result read in a cross-endian context.
That way, when HDF5_ gets the data and byte-swaps it as it is expecting to, it will produce the correct final result.
There is an endianness test in the Makefile and two ZFP_ compressed example datasets for big-endian and little-endian machines to test that cross-endian reads/writes work correctly.

Again, because most CPUs are now little-endian and because ZFP_ became available only after the industry mostly moved away from big-endian, it is highly unlikely that this inefficiency will be triggered.

Finally, *endian-targeting*, which is setting the file datatype for an endianness that is possibly different than the native endianness of the writer, is explicitly disallowed.
For example, data may be produced on a big-endian system, but most consumers will be little-endian.
Therefore, to alleviate downstream consumers from having to always byte-swap, it is desirable to byte-swap to little-endian when the data is written.
However, the juxtaposition of HDF5_'s type conversion and filter operations in a pipeline makes this impractical for the H5Z-ZFP_ filter.
The H5Z-ZFP_ filter will explicitly catch this condition, fail the compression and issue an error message.
