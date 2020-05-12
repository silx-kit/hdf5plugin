.. _endian-issues:

=============
Endian Issues
=============

The ZFP_ library writes an endian-independent stream.

When  reading  ZFP_ compressed  data  on  a  machine with  a  different
endian-ness    than   the   writer,    there   is    an   unnavoidable
inefficiency. Upon reading data from disk and decompressing the read
stream with ZFP_, the correct endian-ness is returned in the result from
ZFP_ before the buffer is handed back to HDF5_ from the decompression
filter. This happens regardless of
reader  and  writer  endian-ness  incompatability.  However,  the HDF5_
library is expecting to get from the decompression filter the endian-ness
of the data as it was stored to to file (typically
that of  the  writer machine)  and  expects to have to byte-swap that
buffer before returning to any endian-incompatible caller. So, in the H5Z-ZFP_ plugin, we wind up having
to  un-byte-swap an already correct result read in a cross-endian context. That way, when
HDF5_  gets the data and byte-swaps it, it will produce the correct result.
There is  an endian-ness  test in  the Makefile and two ZFP_ compressed
example  datasets for  big-endian  and little-endian machines to  test
that cross-endian reads/writes work correctly.

Finally, *endian-targetting*,  that is setting the file  datatype for an
endian-ness that is possibly  different than the native endian-ness of
the  writer (to, for example, alleviate down-stream consumers from having
to byte-swap due to endian incompatability between writer and reader)
is explicitly dis-allowed because it is not an operation that is currently
supported by the HDF5 library.
