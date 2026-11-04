import os
import h5py
import numpy as np
import h5py.h5pl
import h5py.h5z as h5z
import sys
import hdf5plugin


plugin_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build', 'tersecodec/'))
#plugin_dir = os.path.join(sys.prefix, "lib", "hdf5", "plugins")
os.environ["HDF5_PLUGIN_PATH"] = plugin_dir

# Enable HDF5 plugin debugging
os.environ["HDF5_DEBUG"] = "all"

print("HDF5_PLUGIN_PATH:", os.environ.get("HDF5_PLUGIN_PATH"))
print("h5py version:", h5py.__version__)
print("HDF5 version:", h5py.version.hdf5_version)

TERSE_FILTER_ID = 32029

if not h5z.filter_avail(TERSE_FILTER_ID):
    print("Automatic loading failed, trying manual registration...")
    import ctypes
    lib_path = os.path.join(plugin_dir, "libh5terse.so")
    try:
        compression_lib = ctypes.CDLL(lib_path)
        register_filter = compression_lib.register_terse_filter
        register_filter.restype = ctypes.c_int
        if register_filter() < 0:
            raise RuntimeError("Failed to register Terse filter")
        print("Manual registration successful")
    except Exception as e:
        raise RuntimeError(f"Failed to load and register filter: {str(e)}")

if not h5z.filter_avail(TERSE_FILTER_ID):
    raise RuntimeError("TERSE filter not available after registration attempts")

print("TERSE filter is available.")
file_name = "test_terse.h5"
dataset_name = "compressed_data"


data = np.random.choice([0, 1], size=(30, 512, 512), p=[0.5, 0.5]).astype(np.uint32)


print(f"Original data shape: {data.shape}")
print(f"Original data size: {data.nbytes / 1024:.2f} KB")

with h5py.File(file_name, "w") as f:
    space = h5py.h5s.create_simple(data.shape)
    dcpl = h5py.h5p.create(h5py.h5p.DATASET_CREATE)
    dcpl.set_chunk(data.shape)


    numpy_to_type_code = {
        np.int16: 0,    # TYPE_CODE_INT16
        np.uint16: 1,   # TYPE_CODE_UINT16
        np.int32: 2,    # TYPE_CODE_INT32
        np.uint32: 3,   # TYPE_CODE_UINT32
    }

    data_type_code = numpy_to_type_code.get(data.dtype.type)
    if data_type_code is None:
        raise ValueError(f"Data type {data.dtype} not supported by TERSE filter")

    
    dcpl.set_filter(TERSE_FILTER_ID, h5py.h5z.FLAG_OPTIONAL, (data_type_code,))

    dtype = h5py.h5t.py_create(data.dtype)
    dset_id = h5py.h5d.create(f.id, dataset_name.encode(), dtype, space, dcpl=dcpl)
    dset = h5py.Dataset(dset_id)
    dset[...] = data


file_size = os.path.getsize(file_name)
print(f"Compressed HDF5 file size: {file_size / 1024:.2f} KB")

with h5py.File(file_name, "r") as f:
    dset = f[dataset_name]
    read_data = dset[:]

assert np.array_equal(data, read_data), "Data verification failed"

print("Data successfully compressed and decompressed with TERSE filter.")