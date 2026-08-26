"""Sample code that illustrates HDF5 direct chunk write to save HTJ2K compressed data"""

import h5py
import imagecodecs
import numpy as np
from numpy.typing import ArrayLike

FILTER_ID = 32033


def save_htj2k_dataset(
    parent: h5py.Group,
    name: str,
    data: ArrayLike,
    **htj2k_encode_kwargs,
) -> h5py.Dataset:
    """Save data as HTJ2K.

    The htj2k filter only supports chunks with at most 2 non-unity
    dimensions, so arrays with more than 2 dimensions are stored with
    one chunk per 2D frame (the last two dimensions), each compressed
    independently.
    """
    data = np.asarray(data)

    if data.ndim == 0:
        raise ValueError("Scalar data not supported")

    if data.dtype.name not in ("int8", "uint8", "int16", "uint16"):
        raise ValueError(f"Unsupported data type: {data.dtype.name}")

    leading_shape = data.shape[:-2]
    frame_shape = data.shape[-2:]
    dataset = parent.create_dataset(
        name,
        shape=data.shape,
        dtype=data.dtype,
        chunks=(1,) * len(leading_shape) + frame_shape,
        compression=FILTER_ID,
    )
    for leading_indices in np.ndindex(*leading_shape):
        codestream = imagecodecs.htj2k_encode(
            np.atleast_2d(data[leading_indices]), **htj2k_encode_kwargs
        )
        dataset.id.write_direct_chunk(
            leading_indices + (0,) * len(frame_shape), codestream, filter_mask=0
        )
    return dataset


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Save a .npy file as an HTJ2K-compressed HDF5 dataset using direct chunk write."
    )
    parser.add_argument("hdf5_file", help="Path to the HDF5 file to create/append to")
    parser.add_argument("dataset_path", help="Path of the dataset within the HDF5 file")
    parser.add_argument("input_file", help="Path to the input .npy file")
    parser.add_argument(
        "--level",
        type=float,
        default=2**-8,
        help="HTJ2K compression level, forwarded to htj2k_encode (default: 2**-8)",
    )
    args = parser.parse_args()

    input_data = np.load(args.input_file)

    with h5py.File(args.hdf5_file, "a") as h5file:
        save_htj2k_dataset(h5file, args.dataset_path, input_data, level=args.level)
