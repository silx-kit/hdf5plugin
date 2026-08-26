# h5z-htj2k

HDF5 filter plugin for [High-Throughput JPEG2000 compression](https://jpeg.org/jpeg2000/htj2k.html) (a.k.a. HTJ2K).

HTJ2K is an addition to the JPEG 2000 family of International Standards developed by JPEG Committee (ISO/IEC JTC 1/SC 29/WG 1).
It brings an order of magnitude increase in throughput to JPEG 2000 at the expense of slightly reduced coding efficiency.

This version of the compression filter supports:

- Array datasets of integer elements of type: signed or unsigned, 1 or 2 bytes wide (i.e., int8, uint8, int16, uint16).
- Chunk shapes with 1, 2 or 3 non-unity dimensions.
  For chunks with 3 non-unity dimensions, the last non-unity dimension must be 3.

This filter currently accepts no user parameter through HDF5 filter options `cd_values` and compresses data in a reversible way (lossless).
To configure the compression, e.g., to save data in an irreversible way (lossy), compress chunks with a library supporting htj2k and use HDF5 direct chunk write (see [H5Dwrite_chunk](https://support.hdfgroup.org/documentation/hdf5/latest/group___h5_d.html)) to save them in a HDF5 dataset (see [`direct_chunk_write.py` sample code](./examples/direct_chunk_write.py)).

The chunks contain a bare high-throughput jpeg2000 codestream.

This repository contains a reference implementation of the filter based on [OpenJPH](https://github.com/aous72/OpenJPH).

## Filter `cd_values`

This filter currently accepts no user-defined HDF5 filter options `cd_values`.

However, HDF5 filter options `cd_values` are computed automatically and stored by the filter's `set_local` function when creating HDF5 datasets:

- 0: Filter version = 1
- 1: Data type: `(0x0100 if is_bigendian else 0x0000) | (0x80 if is_signed else 0x00) | data_type_size_in_bytes`:
  | Size | Unsigned | Signed |
  |---|---|---|
  | 1 byte | uint8: `0x0001` | int8: `0x0081` |
  | 2 bytes (Little Endian) | uint16: `0x0002` | int16: `0x0082` |
  | 2 bytes (Big Endian) | uint16: `0x0102` | int16: `0x0182` |

  If set to `0x00` or missing, the decompression infers the data type size from the bitdepth stored in the jpeg2000 codestream and expects the HDF5 dataset to be little-endian.
  Other values are not supported by this filter.

- 2: Width
- 3: Height
- 4: Number of components (1 or 3)

For decompression, the first 2 `cd_values` only are used to retrieve the dataset's data type size.
Signedness and dimensions are retrieved from the jpeg2000 codestream.

## Build

```bash
mkdir build && cd build
cmake ..
make
```

### Options

| Option | Default | Description |
|---|---|---|
| `-DDECODE_ONLY=ON` | `OFF` | Build in decode-only mode (encoder disabled). |
| `-DCMAKE_BUILD_TYPE=<type>` | `Release` | `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel`. |
| `-DHDF5_PLUGIN_INSTALL_DIR=<path>` | `<libdir>/hdf5/plugin` | Override the HDF5 plugin install directory. |
| `-DBACKEND_INCLUDE_DIR=<path>` | *(empty)* | Directory containing the backend's headers. Must be set together with `BACKEND_LIB_DIR`, or not at all. Default: Use vendored openjph library |
| `-DBACKEND_LIB_DIR=<path>` | *(empty)* | Directory containing the backend's shared libraries. Must be set together with `BACKEND_INCLUDE_DIR`, or not at all. Default: Use vendored openjph library. |

## Install

```bash
make install
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to set up a development environment, format and lint the code and run the test suite.

## License

- This hdf5 compression filter plugin is available under the [MIT license](LICENSE).
- It uses [OpenJPH](https://github.com/aous72/OpenJPH) which is available under the [BSD 2-Clause License](vendored/OpenJPH/LICENSE).

## Credits

- A. Mirone for benchmark&evaluation of jpeg2000 libraries
- Existing HDF5 filters: bitshuffle, blosc, bzip2, zfp
