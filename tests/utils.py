from collections.abc import Generator

import h5py
import htj2k_filter
import imagecodecs
import numpy as np


def _reshape_unity_dimensions(data: np.ndarray) -> np.ndarray:
    """Reshape data by removing unity dimensions and ensuring at least 2D

    This makes data shape suited for htj2k_encode
    """
    if data.ndim >= 3:  # Collapse unity dimensions
        shape = tuple(dim for dim in data.shape if dim != 1)
        if len(shape) <= 1:  # Make it 2D
            if data.shape[-1] <= 1:
                shape = (data.size, 1)
            else:
                shape = (1, data.size)
        return data.reshape(shape)
    else:
        return np.atleast_2d(data)


def htj2k_reshape_and_encode(data: np.ndarray, **htj2k_encode_kwargs) -> bytes:
    """Reshape nD array by removing unity dimensions and encode"""
    reshaped_data = _reshape_unity_dimensions(data)
    return imagecodecs.htj2k_encode(reshaped_data, **htj2k_encode_kwargs)


def _get_htj2k_filter_options(dataset: h5py.Dataset) -> tuple[int, ...] | None:
    create_plist = dataset.id.get_create_plist()
    for index in range(create_plist.get_nfilters()):
        filter_id, _, filter_options, _ = create_plist.get_filter(index)
        if filter_id == htj2k_filter.FILTER_ID:
            return filter_options
    return None


def _iter_chunks(dataset: h5py.Dataset) -> Generator[tuple[int, bytes]]:
    for index in range(dataset.id.get_num_chunks()):
        chunk_info = dataset.id.get_chunk_info(index)
        yield dataset.id.read_direct_chunk(chunk_info.chunk_offset)


def _assert_codestream_chunk(dataset: h5py.Dataset):
    """Check that chunk starts with j2k codestream magic and is compressed"""
    for filter_mask, chunk in _iter_chunks(dataset):
        assert filter_mask == 0, "A chunk does not apply all filters"
        assert chunk[:2] == b"\xff\x4f", "A chunk does not start with codestream magic"


def _assert_filter_options(dataset: h5py.Dataset, data: np.ndarray):
    options = _get_htj2k_filter_options(dataset)
    assert options is not None
    assert options[0] == htj2k_filter.FILTER_VERSION
    assert options[1] == htj2k_filter.dtype_cd_value(
        byteorder=data.dtype.byteorder,
        is_signed=np.issubdtype(data.dtype, np.signedinteger),
        itemsize=data.itemsize,
    )

    # Expected shape encoded in codestream
    j2k_shape = _reshape_unity_dimensions(data).shape
    assert options[2] == j2k_shape[1], f"{options[2]} != {j2k_shape[1]}"  # Width
    assert options[3] == j2k_shape[0], f"{options[3]} != {j2k_shape[0]}"  # Height
    if len(j2k_shape) == 3:
        assert options[4] == j2k_shape[2], (
            f"{options[4]} != {j2k_shape[2]}"
        )  # N components
    else:
        assert options[4] == 1


def assert_compress(
    data: np.ndarray,
    atol: float = 0,
    rmse_tolerance: float | None = None,
):
    """Write data through htj2k filter, reads with hdf5 direct chunk read and decompress"""
    with h5py.File("in-memory.h5", mode="w", driver="core", backing_store=False) as h5f:
        dataset = h5f.create_dataset(
            "data",
            data=data,
            chunks=data.shape,
            compression=htj2k_filter.FILTER_ID,
        )
        _assert_filter_options(dataset, data)
        _, codestream = dataset.id.read_direct_chunk((0,) * len(data.shape))
        decompressed = imagecodecs.htj2k_decode(codestream, planar=False)

    reshaped_data = _reshape_unity_dimensions(data)
    diff = decompressed.astype(np.float64) - reshaped_data.astype(np.float64)
    max_diff = np.max(np.abs(diff))
    assert max_diff <= atol, f"max diff: {max_diff} > {atol}"
    if rmse_tolerance is not None:
        rmse = np.sqrt(np.mean(diff**2))
        assert rmse <= rmse_tolerance, f"rmse: {rmse} > {rmse_tolerance}"


def assert_decompress(
    codestream: bytes,
    expected: np.ndarray,
    atol: float = 0,
    rmse_tolerance: float | None = None,
):
    """Write codestream with hdf5 direct chunk write and readback through jpeg200 filter"""
    with h5py.File("in-memory.h5", mode="w", driver="core", backing_store=False) as h5f:
        dataset = h5f.create_dataset(
            "data",
            shape=expected.shape,
            dtype=expected.dtype,
            chunks=expected.shape,
            compression=htj2k_filter.FILTER_ID,
        )
        _assert_filter_options(dataset, expected)
        dataset.id.write_direct_chunk(
            (0,) * len(expected.shape), codestream, filter_mask=0
        )
        decompressed = dataset[()]

    diff = decompressed.astype(np.float64) - expected.astype(np.float64)
    max_diff = np.max(np.abs(diff))
    assert max_diff <= atol, f"max diff: {max_diff} > {atol}"
    if rmse_tolerance is not None:
        rmse = np.sqrt(np.mean(diff**2))
        assert rmse <= rmse_tolerance, f"rmse: {rmse} > {rmse_tolerance}"


def assert_roundtrip(data: np.ndarray, atol: int = 0):
    """Roundtrip data through jpeg200 filter"""
    with h5py.File("in-memory.h5", mode="w", driver="core", backing_store=False) as h5f:
        dataset = h5f.create_dataset(
            "data",
            data=data,
            chunks=data.shape,
            compression=htj2k_filter.FILTER_ID,
        )
        _assert_filter_options(dataset, data)
        _assert_codestream_chunk(dataset)

        decompressed = dataset[()]

    max_diff = np.max(np.abs(decompressed.astype(np.float64) - data.astype(np.float64)))
    assert max_diff <= atol, f"max diff: {max_diff} > {atol}"
