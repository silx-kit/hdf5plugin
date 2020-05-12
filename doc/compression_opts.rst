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

zfp
...

For more information, see `zfp modes <https://zfp.readthedocs.io/en/latest/modes.html>`_ and `hdf5-zfp generic interface <https://h5z-zfp.readthedocs.io/en/latest/interfaces.html#generic-interface>`_.

The first value of *compression_opts* is **mode**.
The following values depends on the value of **mode**:

- *Fixed-rate* mode:       (1, 0, **rateHigh**, **rateLow**, 0, 0)
  Rate, i.e., number of compressed bits per value, as a double stored as:

  - **rateHigh**: High 32-bit word of the rate double.
  - **rateLow**: Low 32-bit word of the rate double.

- *Fixed-precision* mode:  (2, 0, **prec**, 0, 0, 0)

  - **prec**: Number of uncompressed bits per value.

- *Fixed-accuracy* mode:   (3, 0, **accHigh**, **accLow**, 0, 0)
  Accuracy, i.e., absolute error tolerance, as a double stored as:

  - **accHigh**: High 32-bit word of the accuracy double.
  - **accLow**: Low 32-bit word of the accuracy double.

- *Expert* mode:     (4, 0, **minbits**, **maxbits**, **maxprec**, **minexp**)

  - **minbits**: Minimum number of compressed bits used to represent a block.
  - **maxbits**: Maximum number of bits used to represent a block.
  - **maxprec**: Maximum number of bit planes encoded.
  - **minexp**: Smallest absolute bit plane number encoded.

- *Reversible* mode: (5, 0, 0, 0, 0, 0)

