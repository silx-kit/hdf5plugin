"""Implementation of a pure Python HDF5 htj2k filter

- compression not implemented: use direct chunk write
- decompression using OpenJPH via imagecodecs

Reference: https://github.com/h5py/h5py/blob/master/h5py/tests/test_h5z.py
"""

import ctypes
import sys
from ctypes import POINTER, c_char_p, c_int, c_int64, c_size_t, c_uint, c_void_p
from typing import Literal, Sequence, cast

import h5py
import imagecodecs
import numpy as np

__all__ = [
    "FILTER_ID",
    "FILTER_VERSION",
    "dtype_cd_value",
    "htj2k_filter",
    "register",
]


FILTER_ID = 32033
FILTER_VERSION = 1

# cd_values layout from H5Zhtj2k.c
CD_INDEX_VERSION = 0
CD_INDEX_DTYPE = 1
CD_INDEX_WIDTH = 2
CD_INDEX_HEIGHT = 3
CD_INDEX_NCOMPS = 4
CD_NELMTS_COMPRESS_REQUIRED = 5


def dtype_cd_value(
    byteorder: Literal["<", ">", "="],
    is_signed: bool,
    itemsize: int,
) -> int | None:
    """Equivalent of H5Z_HTJ2K_DTYPE_WITH_ENDIANNESS(order, is_signed, nbytes)"""
    if itemsize not in (1, 2):
        return None
    if byteorder == "=":
        byteorder = "<" if sys.byteorder == "little" else ">"
    endianness_bit = 0x0100 if itemsize > 1 and byteorder == ">" else 0x000
    return endianness_bit | (0x80 if is_signed else 0x00) | itemsize


def dtype_cd_value_to_str(dtype_cd_value: int) -> str | None:
    if dtype_cd_value == 0:
        return None

    sign = "" if dtype_cd_value & 0x80 else "u"
    itemsize = dtype_cd_value & 0x7F
    if itemsize not in (1, 2):
        return None
    return f"{sign}int{itemsize * 8}"


def _dtype_byteorder(dtype_cd_value: int) -> Literal["<", ">"]:
    return ">" if dtype_cd_value & 0x0100 else "<"


_H5ZFuncT = ctypes.CFUNCTYPE(
    c_size_t,  # restype
    # argtypes
    c_uint,  # flags
    c_size_t,  # cd_nelemts
    POINTER(c_uint),  # cd_values
    c_size_t,  # nbytes
    POINTER(c_size_t),  # buf_size
    POINTER(c_void_p),  # buf
)

_H5ZSetLocalFuncT = ctypes.CFUNCTYPE(
    c_int,  # restype
    # argtypes
    c_int64,  # dcpl_id
    c_int64,  # type_id
    c_int64,  # space_id
)


class _H5ZClass2T(ctypes.Structure):
    """H5Z_class2_t structure defining a filter"""

    _fields_ = [
        ("version", c_int),
        ("id_", c_int),
        ("encoder_present", c_uint),
        ("decoder_present", c_uint),
        ("name", c_char_p),
        ("can_apply", c_void_p),
        ("set_local", _H5ZSetLocalFuncT),
        ("filter_", _H5ZFuncT),
    ]


_H5Z_CLASS_T_VERS = h5py.h5z.CLASS_T_VERS if h5py.version.version_tuple >= (3, 9) else 1


# Function not exposed by h5py
if sys.platform.startswith("win"):
    _libhdf5 = ctypes.cdll.LoadLibrary("hdf5")
else:
    _libhdf5 = ctypes.CDLL(h5py.h5z.__file__)

_libhdf5.H5allocate_memory.restype = ctypes.c_void_p
_libhdf5.H5allocate_memory.argtypes = [ctypes.c_size_t, ctypes.c_bool]

_libhdf5.H5free_memory.argtypes = [ctypes.c_void_p]

_libhdf5.H5Pmodify_filter.restype = c_int  # herr_t
_libhdf5.H5Pmodify_filter.argtypes = [
    c_int64,  # hid_t plist_id
    c_int,  # H5Z_filter_t filter
    c_uint,  # unsigned int flags
    c_size_t,  # size_t cd_nelmts
    POINTER(c_uint),  # const unsigned int cd_values[]
]


def _H5Pmodify_filter(
    plist_id: int, filter_: int, flags: int, cd_nelmts: int, cd_values: Sequence[int]
) -> int:
    cd_values_array = (c_uint * len(cd_values))(*cd_values)
    return cast(
        int,
        _libhdf5.H5Pmodify_filter(plist_id, filter_, flags, cd_nelmts, cd_values_array),
    )


@_H5ZFuncT
def _filter_callback(
    flags: int, cd_nelemts: int, cd_values, nbytes: int, buf_size, buf
):
    version = cd_values[CD_INDEX_VERSION] if cd_nelemts > CD_INDEX_VERSION else 0
    dtype = cd_values[CD_INDEX_DTYPE] if cd_nelemts > CD_INDEX_DTYPE else 0

    if version > FILTER_VERSION:
        print("Incompatible JPEG2000 filter version", file=sys.stderr)
        return 0

    if flags & h5py.h5z.FLAG_REVERSE:  # Decompression
        codestream = ctypes.string_at(buf[0], nbytes)
        data = np.ascontiguousarray(imagecodecs.htj2k_decode(codestream, planar=False))

        if dtype != 0:
            dtype_str = dtype_cd_value_to_str(dtype)
            if dtype_str is None:
                print(f"Unsupported dtype: {dtype}", file=sys.stderr)
                return 0
            data = data.astype(dtype_str)

        if data.itemsize > 1 and _dtype_byteorder(dtype) != data.dtype.str[0]:
            data.byteswap(inplace=True)

        ouput_buffer_size = data.nbytes
        output_buffer = c_void_p(_libhdf5.H5allocate_memory(ouput_buffer_size, False))
        ctypes.memmove(output_buffer, data.ctypes.data, data.nbytes)

    else:  # Compression
        if cd_nelemts < CD_NELMTS_COMPRESS_REQUIRED:
            print("Missing some cd_values for compression", file=sys.stderr)
            return 0

        dtype_str = dtype_cd_value_to_str(dtype)
        if dtype_str is None:
            print(
                f"htj2k compression not implemented for dtype: {dtype}", file=sys.stderr
            )
            return 0

        width = cd_values[CD_INDEX_WIDTH]
        height = cd_values[CD_INDEX_HEIGHT]
        ncomps = cd_values[CD_INDEX_NCOMPS]
        if ncomps == 1:
            shape = height, width
        else:
            shape = height, width, ncomps
        data = np.frombuffer(
            ctypes.string_at(buf[0], buf_size[0]), dtype=dtype_str
        ).reshape(shape)

        codestream = imagecodecs.htj2k_encode(data, planar=False, reversible=True)

        ouput_buffer_size = len(codestream)
        output_buffer = c_void_p(_libhdf5.H5allocate_memory(ouput_buffer_size, False))
        ctypes.memmove(output_buffer, codestream, ouput_buffer_size)

    _libhdf5.H5free_memory(buf[0])

    buf_size[0] = ouput_buffer_size
    buf[0] = output_buffer
    return ouput_buffer_size


@_H5ZSetLocalFuncT
def _set_local_callback(dcpl_id: int, type_id: int, space_id: int) -> int:
    dcpl = h5py.h5p.PropDCID(dcpl_id)
    dcpl.locked = True
    type_ = h5py.h5t.TypeIntegerID(type_id)
    type_.locked = True
    space = h5py.h5s.SpaceID(space_id)
    space.locked = True

    flags, _, _ = dcpl.get_filter_by_id(FILTER_ID)

    # Compute non-unity dimensions in chunk
    dims = space.get_simple_extent_dims()
    dims_used = [d for d in dims if d > 1]

    if len(dims_used) == 0:
        width, height, ncomps = 1, 1, 1
    elif len(dims_used) == 1:
        if dims[-1] <= 1:
            width, height, ncomps = 1, dims_used[0], 1
        else:
            width, height, ncomps = dims_used[0], 1, 1
    elif len(dims_used) == 2:
        width, height, ncomps = dims_used[1], dims_used[0], 1
    elif len(dims_used) == 3:
        if dims_used[2] != 3:
            print(
                "For chunks with 3 non-unity dimensions, the last dimension must be equal to 3",
                file=sys.stderr,
            )
            return -1
        width, height, ncomps = dims_used[1], dims_used[0], dims_used[2]
    else:
        print("Unsupported number of dimensions", file=sys.stderr)
        return -1

    # Retrieve data type
    dclass = type_.get_class()
    if dclass != h5py.h5t.INTEGER:
        print(f"Unsupported datatype class: {dclass}", file=sys.stderr)
        return -1

    dorder = type_.get_order()
    native_order = h5py.h5t.ORDER_LE if sys.byteorder == "little" else h5py.h5t.ORDER_BE
    if dorder != native_order:
        print(f"Unsupported datatype order: {dorder}", file=sys.stderr)
        return -1

    dsize = type_.get_size()
    dtype = dtype_cd_value(
        "<" if dorder == h5py.h5t.ORDER_LE else ">",
        type_.get_sign() == h5py.h5t.SGN_2,
        dsize,
    )
    if dtype is None:
        print(f"Unsupported datatype size; {dsize}", file=sys.stderr)
        return -1

    cd_values = [0] * CD_NELMTS_COMPRESS_REQUIRED
    cd_values[CD_INDEX_VERSION] = FILTER_VERSION
    cd_values[CD_INDEX_DTYPE] = dtype
    cd_values[CD_INDEX_WIDTH] = width
    cd_values[CD_INDEX_HEIGHT] = height
    cd_values[CD_INDEX_NCOMPS] = ncomps

    result = _H5Pmodify_filter(dcpl_id, FILTER_ID, flags, len(cd_values), cd_values)
    if result < 0:
        print("H5Pmodify_filter failed", file=sys.stderr)
        return -1

    return 1


htj2k_filter = _H5ZClass2T(
    version=_H5Z_CLASS_T_VERS,
    id_=FILTER_ID,
    encoder_present=1,
    decoder_present=1,
    name=b"htj2k",
    can_apply=None,
    set_local=_set_local_callback,
    filter_=_filter_callback,
)


def register(force: bool = False) -> bool:
    if h5py.h5z.filter_avail(FILTER_ID):
        if not force:
            return True
        h5py.h5z.unregister_filter(FILTER_ID)

    return h5py.h5z.register_filter(ctypes.addressof(htj2k_filter))
