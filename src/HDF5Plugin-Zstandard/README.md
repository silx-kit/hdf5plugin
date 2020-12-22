# HDF5Plugin-Zstandard

*HDF5* is a data model, library, and file format for storing and
managing data. It supports an unlimited variety of datatypes, and is
designed for flexible and efficient I/O and for high volume and
complex data. HDF5 is portable and is extensible, allowing
applications to evolve in their use of HDF5. The HDF5 Technology suite
includes tools and applications for managing, manipulating, viewing,
and analyzing data in the HDF5 format.

https://support.hdfgroup.org/HDF5/

---

*Zstandard* is a real-time compression algorithm, providing high
compression ratios. It offers a very wide range of compression/speed
trade-off, while being backed by a very fast decoder.

http://www.zstd.net

---

This repository provides an implementation of Zstandard compression
filter plugin for HDF5 with the assigned filter code 32015.

---

## Build

This plugin can be built with cmake and installed as shared library to `/usr/local/hdf5/lib/plugin` (or a custom path).

```bash
cmake .
make
sudo make install
```
