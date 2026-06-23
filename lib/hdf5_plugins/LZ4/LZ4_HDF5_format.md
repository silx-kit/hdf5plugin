# LZ4 HDF5 Format Description

## Introduction

This document describes the LZ4 HDF5 format used in HDF5 Filter ID: `32004`
This is a more detailed version of [_HDF5_LZ4.pdf_](https://github.com/dectris/HDF5Plugin/blob/master/HDF5_LZ4.pdf)

No client data values are required to decode this format.

This format is not compatible with [LZ4 Frame](https://github.com/lz4/lz4/blob/v1.10.0/doc/lz4_Frame_format.md)
or [LZ4 Block](https://github.com/lz4/lz4/blob/v1.10.0/doc/lz4_Block_format.md) formats, but uses the LZ4 Block format internally.

## General Structure

There is a 12 byte header followed by a number of blocks.

Each block starts with 4 byte header containing the compressed size of the block.

Blocks may be compressed or not compressed.
Each block can be decompressed independently of the other blocks.

Only the blocks required to store the original data are produced.
If the original size is zero, there are no blocks.

| Orig Size | Block Size | C Size 0 | Data 0   | (...) | C Size N | Data N   |
|:---------:|:----------:| -------- | -------- | ----- | -------- | -------- |
| 8 bytes   | 4 bytes    | 4 bytes  | C Size 0 |       | 4 bytes  | C Size N |

### Original Size

Total decompressed size stored in big endian signed 64 bit format.

### Block Size

Decompressed bytes per block stored in big endian signed 32 bit format.
All blocks must have this decompressed size except the last one, which might be smaller.

Block size must be greater than zero.

### Compressed Size

The size of the data field stored in big endian signed 32 bit format.

If this value is equal to the known decompressed size of the block, the data is stored
not compressed. Otherwise, the data is in the [LZ4 Block](https://github.com/lz4/lz4/blob/v1.10.0/doc/lz4_Block_format.md) format.
