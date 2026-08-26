import sys

import h5py
import htj2k_filter
import imagecodecs
import numpy as np
import pytest
from utils import (
    assert_compress,
    assert_decompress,
    assert_roundtrip,
    htj2k_reshape_and_encode,
)


def _out_dtype(bit_depth, signed):
    """Smallest numpy integer dtype that holds a `bit_depth`-bit sample."""
    if signed:
        for b, dt in ((8, np.int8), (16, np.int16)):
            if bit_depth <= b:
                return dt
    else:
        for b, dt in ((8, np.uint8), (16, np.uint16)):
            if bit_depth <= b:
                return dt
    raise ValueError("bit_depth must be in 1..16")


def make_bitdepth_test_image(
    bit_depth: int,
    shape: tuple[int, int] = (256, 512),
    signed: bool = False,
    lsb_bits: int = 1,
    tile: int = 64,
    seed: int = 0,
) -> np.ndarray:
    """
    Build a test image for probing a codec's *effective* bit depth via a
    lossless round-trip + bit-exact diff. The pattern combines three probes:

      1. Coarse full-range ramp along columns  -> magnitude ceiling / internal
         buffer width. Errors clustered at high magnitudes => buffer too narrow.
      2. White noise confined to the lowest `lsb_bits` bits -> LSB preservation /
         reversible-transform headroom. This is the worst case: the wavelet
         cannot decorrelate white noise, so it fully stresses coder precision.
         LSBs stripped everywhere => effective depth below the claimed depth.
      3. Constant extremal tiles at min and max -> clip / saturation at extremes.

    Returns a 2-D numpy array in the smallest integer dtype that fits `bit_depth`.
    """
    if not (1 <= bit_depth <= 32):
        raise ValueError("bit_depth must be in 1..32")
    if not (1 <= lsb_bits <= bit_depth):
        raise ValueError("lsb_bits must be in 1..bit_depth")

    H, W = shape
    rng = np.random.default_rng(seed)
    maxval = (1 << bit_depth) - 1  # raw unsigned B-bit maximum

    # 1) coarse ramp sweeping the full magnitude range across the columns
    ramp = np.rint(np.linspace(0, maxval, W)).astype(np.int64)
    img = np.broadcast_to(ramp, (H, W)).copy()

    # 2) replace the lowest `lsb_bits` bits with uniform white noise
    lsb_mask = (1 << lsb_bits) - 1
    noise = rng.integers(0, lsb_mask + 1, size=(H, W), dtype=np.int64)
    img = (img & ~lsb_mask) | noise  # stays within [0, maxval]

    # 3) constant extremal tiles (applied last so they are truly flat)
    t = min(tile, H, W // 2)
    img[:t, :t] = 0  # all-min tile
    img[:t, -t:] = maxval  # all-max tile

    # 4) map raw B-bit space to signed if requested: [0,max] -> [-2^(B-1), 2^(B-1)-1]
    if signed:
        img = img - (1 << (bit_depth - 1))

    return img.astype(_out_dtype(bit_depth, signed))


@pytest.mark.parametrize("signed", [True, False])
@pytest.mark.parametrize("bitdepth", [4, 8, 12, 16])
def test_compress_bitdepth(signed: bool, bitdepth: int):
    """Test compress image bitdepth full range with noise in least significant bits

    Compress data with the filter, reads with direct chunk access and decompress with imagecodecs
    """
    data = make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1)
    assert_compress(data)


@pytest.mark.parametrize("signed", [True, False])
@pytest.mark.parametrize("bitdepth", [4, 8, 12, 16])
def test_decompress_bitdepth(signed: bool, bitdepth: int):
    """Test bitdepth full range with noise in least significant bits

    Compress data with imagecodecs, save it with direct chunk write and read-back through filter
    """
    data = make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1)
    codestream = imagecodecs.htj2k_encode(data, reversible=True)

    assert_decompress(codestream, data)


@pytest.mark.parametrize("signed", [True, False])
@pytest.mark.parametrize("bitdepth", [4, 8, 12, 16])
def test_roundtrip_bitdepth(signed: bool, bitdepth: int):
    """Test roundtrip image bitdepth full range with noise in least significant bits"""
    data = make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1)
    assert_roundtrip(data)


@pytest.mark.parametrize("dtype", ["int8", "int16", "uint8", "uint16"])
def test_compress_3_components(dtype: str):
    """Test compress image with 3 components

    Compress data with the filter, reads with direct chunk access and decompress with imagecodecs
    """
    signed = np.issubdtype(dtype, np.signedinteger)
    bitdepth = np.dtype(dtype).itemsize
    data = np.transpose(
        [
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=0),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=1),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=2),
        ],
        axes=(1, 2, 0),
    )
    assert_compress(data)


@pytest.mark.parametrize("dtype", ["int8", "int16", "uint8", "uint16"])
def test_decompress_3_components(dtype: str):
    """Test decompress image with 3 components

    Compress data with imagecodecs, save it with direct chunk write and read-back through filter
    """
    signed = np.issubdtype(dtype, np.signedinteger)
    bitdepth = np.dtype(dtype).itemsize
    data = np.transpose(
        [
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=0),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=1),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=2),
        ],
        axes=(1, 2, 0),
    )
    codestream = imagecodecs.htj2k_encode(data, planar=False, reversible=True)
    assert_decompress(codestream, data)


@pytest.mark.parametrize("dtype", ["int8", "int16", "uint8", "uint16"])
def test_roundtrip_3_components(dtype: str):
    """Test roundtrip image with 3 components"""
    signed = np.issubdtype(dtype, np.signedinteger)
    bitdepth = np.dtype(dtype).itemsize
    data = np.transpose(
        [
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=0),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=1),
            make_bitdepth_test_image(bitdepth, signed=signed, lsb_bits=1, seed=2),
        ],
        axes=(1, 2, 0),
    )
    assert_roundtrip(data)


def test_none_native_endianness():
    native_data = np.arange(200, dtype=np.int16).reshape(10, 20)

    none_native_endianness = "<" if sys.byteorder == "big" else ">"
    none_native_data = native_data.astype(f"{none_native_endianness}i2")

    with h5py.File("in-memory.h5", mode="w", driver="core", backing_store=False) as h5f:
        h5f.create_dataset(
            "native_data",
            data=native_data,
            chunks=native_data.shape,
            compression=htj2k_filter.FILTER_ID,
        )

        # None native endianess is not supported
        with pytest.raises(ValueError):
            h5f.create_dataset(
                "none_native_data",
                data=none_native_data,
                chunks=none_native_data.shape,
                compression=htj2k_filter.FILTER_ID,
            )


@pytest.mark.parametrize("dtype", ["int8", "int16", "uint8", "uint16"])
def test_decompress_bitdepth4(dtype: str):
    """Test "small" bitdepth=4 with different possibly larger dtype.

    Compress data with imagecodecs, save it with direct chunk write and read-back through filter
    """
    data = make_bitdepth_test_image(
        bit_depth=4,
        signed=np.issubdtype(dtype, np.signedinteger),
        lsb_bits=1,
    )
    codestream = imagecodecs.htj2k_encode(data, reversible=True)

    assert_decompress(codestream, data.astype(dtype))


@pytest.mark.parametrize("dtype", ["int8", "int16", "uint8", "uint16"])
def test_roundtrip_bitdepth4(dtype: str):
    """Test roundtrip image with "small" bitdepth=4 with different possibly larger dtype."""
    data = make_bitdepth_test_image(
        bit_depth=4,
        signed=np.issubdtype(dtype, np.signedinteger),
        lsb_bits=1,
    )
    assert_roundtrip(data)


SHAPE_TESTS_DTYPE = "uint16"
SHAPE_TEST_DATA_1D = (
    np.iinfo(np.dtype(SHAPE_TESTS_DTYPE)).max
    * 0.5
    * (1 + np.sin(np.linspace(0, 12 * np.pi, 1000)))
).astype(SHAPE_TESTS_DTYPE)

SHAPE_TEST_DATA_2D_SIZE = 256
SHAPE_TEST_DATA_2D = (
    np.iinfo(np.dtype(SHAPE_TESTS_DTYPE)).max
    * 0.5
    * (
        1
        + np.outer(
            np.sin(np.linspace(0, 4 * np.pi, SHAPE_TEST_DATA_2D_SIZE)),
            np.sin(np.linspace(0, 4 * np.pi, SHAPE_TEST_DATA_2D_SIZE)),
        )
    )
).astype(SHAPE_TESTS_DTYPE)

SHAPE_SINGLE_VALUE_TESTS = {
    "shape_1d_single_value": np.array([1], dtype=SHAPE_TESTS_DTYPE),
    "shape_2d_single_value": np.array([[1]], dtype=SHAPE_TESTS_DTYPE),
    "shape_3d_single_value": np.array([[[1]]], dtype=SHAPE_TESTS_DTYPE),
    "shape_4d_single_value": np.array([[[[1]]]], dtype=SHAPE_TESTS_DTYPE),
}


SHAPE_1D_TESTS = {
    "shape_1d_data": SHAPE_TEST_DATA_1D,
    "shape_2d_unity_dim0": SHAPE_TEST_DATA_1D.reshape(1, -1),
    "shape_2d_unity_dim1": SHAPE_TEST_DATA_1D.reshape(-1, 1),
    "shape_3d_unity_dim0_dim1": SHAPE_TEST_DATA_1D.reshape(1, 1, -1),
    "shape_3d_unity_dim0_dim2": SHAPE_TEST_DATA_1D.reshape(1, -1, 1),
    "shape_3d_unity_dim1_dim2": SHAPE_TEST_DATA_1D.reshape(-1, 1, 1),
    "shape_4d_unity_dim0_dim2_dim3": SHAPE_TEST_DATA_1D.reshape(1, -1, 1, 1),
    "shape_4d_unity_dim1_dim2_dim3": SHAPE_TEST_DATA_1D.reshape(-1, 1, 1, 1),
}


SHAPE_2D_TESTS = {
    "shape_1_2": np.array([[0, 1]], dtype=SHAPE_TESTS_DTYPE),
    "shape_2_2": np.array([[0, 1], [1, 0]], dtype=SHAPE_TESTS_DTYPE),
    "shape_3_5": np.array([[0, 1, 2, 1, 0], [1, 0, 0, 1, 2]], dtype=SHAPE_TESTS_DTYPE),
    "shape_2d_data": SHAPE_TEST_DATA_2D,
    "shape_3d_unity_dim0": SHAPE_TEST_DATA_2D.reshape(
        1, SHAPE_TEST_DATA_2D_SIZE, SHAPE_TEST_DATA_2D_SIZE
    ),
    "shape_3d_unity_dim1": SHAPE_TEST_DATA_2D.reshape(
        SHAPE_TEST_DATA_2D_SIZE, 1, SHAPE_TEST_DATA_2D_SIZE
    ),
    "shape_3d_unity_dim2": SHAPE_TEST_DATA_2D.reshape(
        SHAPE_TEST_DATA_2D_SIZE, SHAPE_TEST_DATA_2D_SIZE, 1
    ),
    "shape_4d_unity_dim1_dim2": SHAPE_TEST_DATA_2D.reshape(
        SHAPE_TEST_DATA_2D_SIZE, 1, 1, SHAPE_TEST_DATA_2D_SIZE
    ),
}


ALL_SHAPE_TESTS = {**SHAPE_SINGLE_VALUE_TESTS, **SHAPE_1D_TESTS, **SHAPE_2D_TESTS}


@pytest.mark.parametrize("data", ALL_SHAPE_TESTS.values(), ids=ALL_SHAPE_TESTS.keys())
def test_compress_shape_reversible(data: np.ndarray):
    assert_compress(data)


@pytest.mark.parametrize("data", ALL_SHAPE_TESTS.values(), ids=ALL_SHAPE_TESTS.keys())
def test_decompress_shape_reversible(data: np.ndarray):
    codestream = htj2k_reshape_and_encode(data, reversible=True)
    assert_decompress(codestream, data)


@pytest.mark.parametrize(
    "data", SHAPE_SINGLE_VALUE_TESTS.values(), ids=SHAPE_SINGLE_VALUE_TESTS.keys()
)
def test_decompress_shape_single_value_lossy(data: np.ndarray):
    codestream = htj2k_reshape_and_encode(data, level=2**-8, reversible=False)
    assert_decompress(codestream, data, atol=2)


@pytest.mark.parametrize("data", SHAPE_1D_TESTS.values(), ids=SHAPE_1D_TESTS.keys())
def test_decompress_shape_1d_lossy(data: np.ndarray):
    codestream = htj2k_reshape_and_encode(data, level=2**-8, reversible=False)
    assert_decompress(codestream, data, atol=160, rmse_tolerance=10)


@pytest.mark.parametrize("data", SHAPE_2D_TESTS.values(), ids=SHAPE_2D_TESTS.keys())
def test_decompress_shape_2d_lossy(data: np.ndarray):
    codestream = htj2k_reshape_and_encode(data, level=2**-8, reversible=False)
    assert_decompress(codestream, data, atol=227, rmse_tolerance=21)


@pytest.mark.parametrize("data", ALL_SHAPE_TESTS.values(), ids=ALL_SHAPE_TESTS.keys())
def test_roundtrip_shape(data: np.ndarray):
    assert_roundtrip(data)


def test_decompress_16M():
    data = make_bitdepth_test_image(
        shape=(4096, 4096), bit_depth=16, signed=False, lsb_bits=2
    )
    codestream = imagecodecs.htj2k_encode(data, reversible=True)

    assert_decompress(codestream, data)


def test_roundtrip_16M():
    data = make_bitdepth_test_image(
        shape=(4096, 4096), bit_depth=16, signed=False, lsb_bits=2
    )
    assert_roundtrip(data)
