# TRPX V2.0 data compression algorithm

This repository provides two related components:

- HDF5 **terse** filter – an HDF5 plugin that compresses diffraction data using the next-generation TERSE/PROLIX (TRPX) algorithm.
- **Pyterse** – a Python package that provides direct Python bindings (via pybind11) to the TERSE/PROLIX library.

Both are powered by the same TERSE/PROLIX algorithm.
You can build only the HDF5 filter, only the Pyterse Python package, or both. 

# Pyterse python package


The pyterse python package provides Python bindings for the C++ TERSE/PROLIX(TRPX) compression algorithm scheme ([https://github.com/senikm/trpx](https://github.com/senikm/trpx)). This version of TRPX is parallel, has better memory management, and compression modes.

## Prerequisites

Before using pyterse, ensure your data meets these requirements:
- Signed or unsigned integral type data
- Grayscale data
- Preferably has high dynamic range

## Installation

Create a virtual environment:
```bash
conda create -n pyterse python pip numpy pillow
conda activate pyterse
```

Install the package:
```python
pip install pyterse
```

## Usage guide

### Basic operations

#### Creating a Terse object

There are multiple ways to create a Terse object:

```python
import pyterse

# Empty constructor
terse = pyterse.Terse()

# From NumPy array
terse = pyterse.Terse(data)  # data can be nD NumPy array or slice

# With custom compression mode
terse = pyterse.Terse(data, pyterse.TerseMode.SIGNED)  # Available modes: SIGNED, UNSIGNED, SMALL_UNSIGNED, DEFAULT
```

#### Inserting and managing data

Add data to an existing Terse object:
```python
# Append data at the end
terse.push_back(data)  # Data must match existing shape (terse.dim())

# Insert at specific position
terse.insert(pos, data)  # pos is the frame index
```

#### File operations

Save and load compressed data:
```python
# Save to file
terse.save('filename.trpx')

# Load from file
loaded_terse = pyterse.Terse.load('filename.trpx')
```

#### Data Decompression

Decompress data:
```python
# Decompress all data
decompressed_data = terse.prolix()

# Decompress specific frame
frame = terse.at(0)
decompressed_frame = frame.prolix()
```

#### Metadata Management

```python
# Set metadata for a frame
terse.set_metadata(frame, "metadata string")

# Get metadata from a frame
metadata = terse.metadata(frame)
```

#### Utility Methods

```python
# Data information
terse.dim()               # Dimensions of one frame
terse.size              # Number of elements per frame
terse.number_of_frames  # Number of frames
terse.number_of_bytes   # Size in bytes of compressed data
terse.bits_per_val      # Bits used per value
terse.is_signed         # Whether data is signed

# Data management
terse.erase(pos)          # Remove frame at position
terse.shrink_to_fit()     # Optimize memory usage

# Compression settings
terse.set_block_size(size)  # Set compression block size (before adding frames)
terse.set_fast(bool)        # Toggle fast compression mode
terse.set_small(bool)       # Toggle small data optimization
terse.dop()                 # Degree of parallelism
terse.set_dop(value)        # Set degree of parallelism (0.0 to 1.0)

# Float data compression
terse.set_fractional_precision (value)
terse.fractional_precision() 
```

## Building from source

```bash
conda create -n pyterse python pip numpy pillow
conda activate pyterse
git clone https://github.com/senikm/trpx2.0.git
cd trpx2.0
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
make
python3 py_tests/pyterse_test.py  # Will run unittests
```

# HDF5 tersecodec

## Prerequisites

Before proceeding, ensure you have the following installed on your system:

1. **Create a new conda environment**

```bash
conda create -n hdf5_terse_env python=3.10
conda activate hdf5_terse_env
```
2. **Install CMake, HDF5 and other dependencies**

```bash
conda install cmake hdf5 numpy h5py hdf5plugin pillow
cmake --version
``` 

3. **Clone the repository and build the filter**

```
git clone https://github.com/senikm/trpx2.0.git
cd trpx2.0
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
make
```
Note: The CMakeLists.txt provided uses the CONDA_PREFIX environment variable to locate HDF5. Ensure the Conda environment is active when running CMake.

4. **Running performance tests**

> Run the test.py in py_tests folder.
```
python3 py_tests/filter_test.py
```
Note: The conda environment is activated and by default the script loads the filter from `build/src/terse.so`.

> Sample output

```sql
HDF5_PLUGIN_PATH: None
h5py version: 3.12.1
HDF5 version: 1.14.3
TERSE filter registered successfully with ID 32029
TERSE filter is available.
Original data shape: (30, 512, 512)
Original data size: 30720.00 KB
Compressed HDF5 file size: 1044.42 KB
Data successfully compressed and decompressed with TERSE filter.
```

5. **You can make the filter path permanent**

```
echo 'export HDF5_PLUGIN_PATH=/path/to/hdf5_filter/build/src' >> ~/.bashrc
source ~/.bashrc
```

# Fiji/ImageJ plugin for .trpx format files


> For compilation, use the Java version that came with Fiji, to ensure Java compatibility. Also make sure the ij-1.??.jar package is included in the compilation:
> For example, compile with:

```bash
    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/javac -cp /Applications/Fiji.app/jars/ij-1.53t.jar TRPX_Reader.java
```
> Then create the .jar files with:
```bash
    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/jar -cvf Terse_Reader.jar TRPX_Reader*.class
```
> Then copy Terse_Reader.jar to the "plugins" directory of Fiji:
```bash
    cp TRPX_Reader.jar /Applications/Fiji.app/plugins/.
```
> Then restart Fiji, and Terse Reader is in the plugins menu.

# Reading TRPX files with DIALS

> Please, install Pyterse package with the following command before reading single TRPX files in DIALS:

```bash
dials.python -m pip install pyterse
```

## Citation

**If you use our software in your research, please cite our paper using the following BibTeX entry**:

```bibtex
@article{MATINYAN2026114298,
    title = {TRPXv2.0: superfast, parallel compression of diffraction patterns and images, with native Python and HDF5 support},
    journal = {Ultramicroscopy},
    volume = {281},
    pages = {114298},
    year = {2026},
    issn = {0304-3991},
    doi = {https://doi.org/10.1016/j.ultramic.2025.114298},
    url = {https://www.sciencedirect.com/science/article/pii/S0304399125001962},
    author = {S. Matinyan and P. Filipcik and D.G. Waterman and C.D. Owen and J.P. Abrahams}
}
```