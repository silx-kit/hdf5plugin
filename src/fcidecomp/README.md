# EUMETSAT ``fcidecomp`` software

The ``fcidecomp`` software enables decompression of JEPG-LS netCDF files via netCDF-C and netCDF-Java based softwares,
various Python libraries and the EUMETSAT Data-Tailor software.

## Supported platforms and installation

The ``fcidecomp`` software can be installed on:

- RockyLinux 8 64-bit (also from source code)
- Linux Ubuntu 18.04 LTS 64-bit
- Linux Ubuntu 20.04 LTS 64-bit (also from source code)
- Windows 10 64-bit


## Installing the ``fcidecomp`` software

Installation procedures are described in the INSTALL file.

## Testing the ``fcidecomp`` software (only for the conda-based installation)

A set of Python unit tests is present to ensure the installed software works correctly. They should be run within the
Conda environment in which the software has been installed.

### Prerequisites

- `pytest`, installed in the Conda environment in which the software has been installed as described 
[here](https://anaconda.org/anaconda/pytest)

Also, the tests depend on the presence of a set of test data, which can be downloaded
[here](<https://gitlab.eumetsat.int/data-tailor/epct-test-data/-/tree/development/MTG/MTGFCIL1>).
Test files should be placed in a directories tree structured as follows (replace $EPCT_TEST_DATA_DIR
with any chosen name):

```BASH
|_$EPCT_TEST_DATA_DIR
  |_MTG
    |_MTGFCIL1
      |_<test_file_1>
      |_<test_file_2>
      |_ ...
```

Once this is done, the environment variable `EPCT_TEST_DATA_DIR` should be set to the full path to the 
`$EPCT_TEST_DATA_DIR` directory.

### Running the tests

Tests can be executed running the following command from within the root directory of the ``fcidecomp`` software repository:

    pytest -vv tests

## Using the ``fcidecomp`` software

The ``fcidecomp`` decompression library can be used in different ways, described in the following sections.

### Use with netCDF4-C tools

### Prerequisites
To use ``fcidecomp`` with ``netCDF4-C``-based tools (e.g. ``nccopy``), two prerequisites are needed:

- the tools must be installed
- the ``HDF5_PLUGIN_PATH`` environment variable must be set as described in the `INSTALL` file.

If using the conda installation, simply activate the environment created above and the prerequisites are met.

If the package has been installed from the source code, set the environment variable and install the
relevant package (``netcdf-4.7.0`` on RockyLinux 8, ``netcdf-bin`` on Ubuntu Linux 20.04).

### Example with ``nccopy``

Once the prerequisites above are met, netCDF4-C tools should be automatically configured to decompress JPEG-LS 
compressed netCDF files. As an example, to decompress a file using `nccopy`, run the following line:

    nccopy -F none $PATH_TO_COMPRESSED_FILE $PATH_TO_DECOMPRESSED_FILE

where:

- `$PATH_TO_COMPRESSED_FILE` is the path to the JPEG-LS compressed file
- `$PATH_TO_DECOMPRESSED_FILE` is the path where the decompressed file should be saved

### Use with `h5py`-based Python libraries (conda install only)

Once the `fcidecomp` Conda package is installed and the Conda environment in which it is installed is activated,
use of the ``fcidecomp`` decompression libraries should be automatically enabled for `h5py`-based Python libraries.

To ensure the ``fcidecomp`` filter is loaded, in a Python shell execute:

    import fcidecomp
    
Now every `h5py`-based Python library, such as `xarray`, will be able to open and read JPEG-LS compressed files without 
further steps.

### Use with netCDF-Java based tools

With netCDF-Java versions greater than 5.5.2, it is possible to open JPEG-LS compressed netCDFs with netCDF-Java based 
tools, such as toolsUI and Panoply, instructing netCDF-Java to use the netCDF-C library for reading purposes. 
To enable this feature:

1. if ``fcidecomp`` has been installed from the source, install the netCDF4 library package (``netcdf-4.7.0``
   on RockyLinux 8, ``libnetcdf-c++4`` on UbuntuLinux 20..04)
2. ensure the file `$HOME/.unidata/nj22Config.xml` exists (if it doesn't, it should be created) and 
   that it contains the following lines:

        <nj22Config>
          <Netcdf4Clibrary>
            <libraryPath>$PATH_TO_NETCDF_LIB_DIR</libraryPath>
            <libraryName>netcdf</libraryName>
            <useForReading>true</useForReading>
          </Netcdf4Clibrary>
        </nj22Config>

    where `$PATH_TO_NETCDF_LIB_DIR` is the path to the directory containing the `netcdf4` library, which:

    - in Linux (conda install), corresponds to `$PATH_TO_CONDA_ENV/lib` 
      with `$PATH_TO_CONDA_ENV` equal to the path to the `conda` environment in which `fcidecomp` is installed.
    - in Windows (conda install), corresponds to `$PATH_TO_CONDA_ENV\Library\lib`
      with `$PATH_TO_CONDA_ENV` equal to the path to the `conda` environment in which `fcidecomp` is installed.
    - in RockyLinux (install from source), corresponds to `/usr/lib64`
    - in Ubuntu 20.04 (install from source), corresponds to `/usr/lib/x86_64-linux-gnu/`

Tested with ToolsUI 5.5.3 on Windows, Panoply 5.1.1 on Linux (known as not working for Panoply for that version in Windows due to a 
Panoply issue).

### Use with the EUMETSAT Data-Tailor software

A plugin enabling the decompression of JPEG-LS Meteosat Third Generation (MTG) products via the ``fcidecomp`` software is
available for the EUMETSAT Data-Tailor software. For further information, refer to the README of its [public GitLab
repository](<https://gitlab.eumetsat.int/open-source>) and the dedicated EUMETSAT confluence page which, once created,
will be a subpage of the [Installing or removing customisation plugins](<https://eumetsatspace.atlassian.net/wiki/spaces/DSDT/pages/378273985/Installing+or+removing+customisation+plugins>)
page.

Inventory Notices
-----------------

Licenses and copyright information for software dependencies up to version 2.0.0
is documented within the ``inventory`` folder.

Files listed under `inventory/items/data_proprietary.ABOUT` are licensed under EUMETSAT Proprietary license.

#### Dependencies
The following dependencies are not included in the package but are required and they will be downloaded at build or compilation time:
* component name, version, SPDX license id, copyright, home_url, comments
* charls, 2.1.0, BSD 3-Clause, - , https://github.com/team-charls/charls, - .
* hdf5, 1.12.* to 1.14.*, BSD 3-Clause, - , https://www.h5py.org/, - .
* h5py, 2.* and 3.6.0, BSD 3-Clause, - , https://www.h5py.org/, - .
* python, 3.9 to 3.12, see https://docs.python.org/3/license.html, - , https://www.python.org/, - .
* zlib, 1.2.13, zlib (http://zlib.net/zlib_license.html), - , https://zlib.net/, - .
* libnetcdf, 4.8.1, MIT , - , https://www.unidata.ucar.edu/software/netcdf/, - .
* libssh2, 1.10.0, - , see https://www.libssh2.org/license.html , https://www.libssh2.org/, - .
* netcdf4, 1.6.2, -, - , https://unidata.github.io/netcdf4-python/, - .

