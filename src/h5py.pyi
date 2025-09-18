from __future__ import annotations

"""mypy stubs for h5py usage in hdf5plugin"""

import ctypes
from collections.abc import Buffer
from types import TracebackType
from typing import Any, BinaryIO, Tuple

from numpy.typing import ArrayLike, DTypeLike, NDArray

class DatasetId:
    def write_direct_chunk(self, offset: tuple[int, ...], data: Buffer) -> None: ...

class Dataset:
    def __getitem__(
        self, key: int | slice | tuple[int | slice, ...]
    ) -> NDArray[Any]: ...
    @property
    def id(self) -> DatasetId: ...

class File:
    def __init__(
        self,
        name: str | BinaryIO,
        mode: str = "r",
        driver: str = None,
        backing_store: bool = None,
    ): ...
    def __enter__(self) -> File: ...
    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: TracebackType | None,
    ) -> bool | None: ...
    def __getitem__(self, key: str) -> Any: ...
    def create_dataset(
        self,
        name: str,
        shape: tuple[int, ...] = None,
        dtype: DTypeLike = None,
        data: ArrayLike = None,
        chunks: tuple[int, ...] | bool | None = None,
        compression: filters.FilterRefBase = None,
    ) -> Any: ...
    def flush(self) -> None: ...

class filters:
    class FilterRefBase:
        filter_id: int | None = None
        filter_options: Tuple[int, ...] = ()

        def __len__(self) -> int: ...

class h5:
    @staticmethod
    def get_libversion() -> tuple[int, ...]: ...

class h5z:
    __file__: str

    @staticmethod
    def filter_avail(filter_id: int) -> bool: ...
    @staticmethod
    def register_filter(filter_: ctypes.c_void_p) -> None: ...
    @staticmethod
    def unregister_filter(filter_id: int) -> bool: ...

class version:
    version_tuple: Tuple[int, ...]
