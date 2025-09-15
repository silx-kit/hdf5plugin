# H5Z-SPERR
This is an HDF5 Plugin for the [SPERR](https://github.com/ncar/sperr) compressor.
It is registered with the HDF Group with a plugin ID of `32028`.

## Build and Install
Needless to say, `H5Z-SPERR` depends on both `HDF5` and `SPERR`. 
After HDF5 and SPERR are installed, `H5Z-SPERR` can be configured and built using `cmake`:
```bash
export HDF5_ROOT=/path/to/your/preferred/HDF5/installation
mkdir build-H5-SPERR && cd build-H5-SPERR
cmake -DSPERR_INSTALL_DIR=/path/to/SPERR/installation     \
      -DCMAKE_INSTALL_PREFIX=/path/to/install/this/plugin \
      /path/to/H5Z-SPERR/source/code
make install
```
The plugin library file `libh5z-sperr.so` will be placed at directory `/path/to/install/this/plugin`.

## Use As a Dynamically Loaded Plugin
Using the [dynamically loaded plugin](https://docs.hdfgroup.org/hdf5/rfc/HDF5DynamicallyLoadedFilters.pdf) mechanism by HDF5,
one may use `H5Z-SPERR` by simply setting the environment variable `HDF5_PLUGIN_PATH` to the directory containing the plugin
library file:
```bash
export HDF5_PLUGIN_PATH=/path/to/install/this/plugin
```
The user program does not need to link to this plugin or SPERR; it only needs to specify the plugin ID of `32028`.

<!--
## Use in NetCDF-4 APIs
`H5Z-SPERR` also facilitates the application of SPERR compression on 
[NetCDF-4 files](https://docs.unidata.ucar.edu/netcdf/NUG/md_filters.html#filters_enable);
one simply needs to define the filter on a variable:
```C
nc_def_var_filter(ncid, varid, 32028, 1, &cd_values);
```
See a complete example [here](https://github.com/NCAR/H5Z-SPERR/blob/main/example/simple_xy_nc4_wr.c).
-->

## Use in Python
`H5Z-SPERR` version `0.1.3` is supported by the Python package [hdf5plugin](https://github.com/silx-kit/hdf5plugin)
since version `5.0.0`.
One can install the package by issuing 
```bash
pip install hdf5plugin [--user]           # using pip
conda install -c conda-forge hdf5plugin   # using conda
```
and use it by importing these two packages:
```python
import h5py         # provide general HDF5 support
import hdf5plugin   # provide HDF5 plugin support
```

## Handling of Missing Values
Simulation models sometimes use a special value (i.e., missing value) to indicate that there's no meaningful value at that specific location.
For example, in an ocean simulation, all the land area is marked by missing values.
The most often seen missing values are either `NaN`, or an extremely large value, such as `1e35` or `-9.9e35`.
When these missing values participate in compression, they easily introduce numeric error and result in data corruption.

`H5Z-SPERR` can handle common missing values with a little help from the user.
Specifically, a user can indicate that there's potentially missing values, 
and `H5Z-SPERR` will use a compact bitmask to keep track of where exactly those missing values are.
`H5Z-SPERR` then replaces them with a value that is friendly to SPERR compression before passing the field to the SPERR compressor.

During decompression, `H5Z-SPERR` fills the original missing value at locations indicated by the compact bitmask.
Specifically, if the original missing values are `NaN`, then `NaN` will be filled. If the orignal missing values
are values with a magnitude larger than `1e35`, then the *first occurance* of such values will be used to fill 
in all missing value locations.

Users use an integer to indicate the potential existance of missing values:
- Mode `0`: no missing values;
- Mode `1`: there are potential `NaN`s;
- Mode `2`: there are potential values with a magnitude larger than `1e35`.

`H5Z-SPERR` behaves accordingly: (`1e35` denotes the *first occurance* of such values)
| Mode      | Actual Input Data    |  Filter Behavior |
|-----------|----------------------|------------------|
| 0         | No `NaN`, no `1e35`  | :heavy_check_mark: Normal SPERR compression |
| 0         | Has `NaN` or `1e35`  | :x: Likely numeric error  |
| 1         | No `NaN`, no `1e35`  | :heavy_check_mark: Normal SPERR compression  |
| 1         | Has `NaN`, no `1e35` | :heavy_check_mark: Normal SPERR compression; `NaN` is restored at its exact locations  |
| 1         | Regardless of `NaN`, has `1e35` |  :x: Likely numeric error  |
| 2         | No `NaN`, no `1e35`  | :heavy_check_mark: Normal SPERR compression  |
| 2         | No `NaN`, has `1e35` | :heavy_check_mark: Normal SPERR compression; `1e35` is restored at its exact locations  |
| 2         | Has `NaN`, regardless of `1e35` | :x: Likely numeric error |

**Final note:** if a variable is indicated to have missing values, but it actually does not, then there's no bitmasks involved thus no storage overhead! 

##  Find `cd_values[]`
To apply SPERR compression using the HDF5 plugin, one needs to specify 1) what compression mode and 2)
what compression quality to use. Supported compression modes and qualities are summarized below:
| Mode No.      | Mode Meaning         | Meaningful Quality Range  |
| ------------- | -------------------- | ------------------------- |
| 1             | Fixed bit-per-pixel (BPP) | 0.0 < quality < 64.0 |
| 2             | Fixed peak signal-to-noise ratio (PSNR) | 0.0 < quality |
| 3             | Fixed point-wise error (PWE)            | 0.0 < quality |

In addition, the rank order needs to be swapped sometimes to achieve the best compression.
For example, if a 2D slice of dimensions `(64, 128)` has the `dim=128` rank being the fastest
varying rank, then this slice needs a "rank order swap" to achieve the best compression.
In general, given dimensions of `(NX, NY, NZ)`, we want the `X` rank to be varying the fastest,
and the `Z` rank to be varying the slowest, before the data is passed to the compressor.
"Rank order swap" helps to achieve it.

The HDF5 libraries takes in these compression parameters as one or more 32-bit `unsigned int` values,
which are named `cd_values[]` in most HDF5 routines.
In the case of `H5Z-SPERR`, there is exactly one `unsigned int` used to carry compression-related information, and 
possibly one more `unsigned int` to indicate the potential existance of missing values.

### Find  `cd_values[]` Using the Programming Interface
Using the HDF5 programming interface, `cd_values[]` carrying the compression parameters are passed
to HDF5 routines such as `H5Pset_filter()`. To find the correct `cd_values[]`, a user
needs to include the header [`h5z-sperr.h`](https://github.com/NCAR/H5Z-SPERR/blob/main/include/h5z-sperr.h)
from this repository
and call the `unsigned int H5Z_SPERR_make_cd_values(int mode, double quality, int swap)` function 
to have these two pieces of information correctly encoded. For example:
```C
int mode = 3;              /* Fixed PWE compression */
double quality = 1e-6;     /* PWE tolerance = 1e-6 */
int swap = 1;              /* Enable rank order swap */
unsigned int cd_values = H5Z_SPERR_make_cd_values(mode, quality, swap);   /* Generate cd_values */
H5Pset_filter(prop, 32028, H5Z_FLAG_MANDATORY, 1, &cd_values);            /* Specify SPERR compression in HDF5 */
```
See a complete example [here](https://github.com/NCAR/H5Z-SPERR/blob/main/utilities/example-3d.c).

### Find `cd_values[]` Using the CLI Tool `generate_cd_values`
After building `H5Z-SPERR`, a command line tool named `generate_cd_values` becomes available to encode 1) SPERR 
compression mode, 2) quality, and 3) if to perform rank order swap
into a single `unsigned int`. The produced value can then be used in other command line tools such as `h5repack`.
In the following example, `generate_cd_values` reports that `268651725u` encodes fixed-rate compression with 
a bitrate of 3.3 bit-per-value, without doing rank order swap.
```Bash
$ ./bin/generate_cd_values 1 3.3
For fixed-rate compression with a bitrate of 3.3, without swapping rank orders,
H5Z-SPERR cd_values = 268651725u (Filter ID = 32028).
Please use this value as a single 32-bit unsigned integer in your applications.
```
Note: an integer produced by `generate_cd_values` can be decoded by another command line tool, `decode_cd_values`,
to show the coded compression parameters.

### Examples
Assume using the `nccopy` tool:
```Bash
# Compress variable VAR0, using fixed-rate compression, bitrate = 3.3, no special handling of missing values.
nccopy -F "VAR0, 268651725u" <input_file> <output_file>
nccopy -F "VAR0, 268651725u, 0" <input_file> <output_file>

# Compress variable VAR1, using fixed-rate compression, bitrate = 3.3. VAR1 might have NaNs!
nccopy -F "VAR1, 268651725u, 1" <input_file> <output_file>

# Compress variable VAR2, using fixed-rate compression, bitrate = 3.3. VAR2 might have values such as 1e35!
nccopy -F "VAR2, 268651725u, 2" <input_file> <output_file> 
```

