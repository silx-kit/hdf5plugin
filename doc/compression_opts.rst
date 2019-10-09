=====================
 Compression options
=====================

Compression filters can be configured with the ``compression_opts`` argument of `h5py.Group.create_dataset <http://docs.h5py.org/en/stable/high/group.html#Group.create_dataset>`_ method by providing a tuple of integers.

The meaning of those integers is filter dependent and is described below.

bitshuffle
..........

compression_opts: (**block_size**, **lz4 compression**)

- **block size**: Number of elements (not bytes) per block.
  It MUST be a mulitple of 8.
  Default: 0 for a block size of about 8 kB.
- **lz4 compression**: 0: disabled (default), 2: enabled.

By default the filter uses bitshuffle, but does NOT compress with LZ4.

blosc
.....

compression_opts: (0, 0, 0, 0, **compression level**, **shuffle**, **compression**)

- First 4 values are reserved.
- **compression level**:
  From 0 (no compression) to 9 (maximum compression).
  Default: 5.
- **shuffle**: Shuffle filter:

  * 0: no shuffle
  * 1: byte shuffle
  * 2: bit shuffle

- **compression**: The compressor blosc ID:

  * 0: blosclz (default)
  * 1: lz4
  * 2: lz4hc
  * 3: snappy
  * 4: zlib
  * 5: zstd

By default the filter uses byte shuffle and blosclz.

lz4
...

compression_opts: (**block_size**,)

- **block size**: Number of bytes per block.
  Default 0 for a block size of 1GB.
  It MUST be < 1.9 GB.

