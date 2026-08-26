from pathlib import Path

import h5py
import numpy as np
import pytest

DATA_PATH = (Path(__file__).parent / "data").resolve()


@pytest.mark.decode_only
@pytest.mark.parametrize("filename", ["bamboo_hercules.h5", "bamboo_hercules_be.h5"])
def test_bamboo_hercules(filename: str):
    with h5py.File(DATA_PATH / filename) as h5f:
        ref_data = h5f["raw"][()]
        decompressed_data = h5f["htj2k"][()]
        expected_rmse = h5f["htj2k"].attrs["RMSE"]
        expected_max_error = h5f["htj2k"].attrs["MAX_ABS_ERROR"]

    diff = decompressed_data.astype(np.float64) - ref_data.astype(np.float64)

    rmse = float(np.sqrt(np.mean(diff * diff)))
    rmse_tolerance = 0.01 * expected_rmse
    assert rmse <= expected_rmse + rmse_tolerance, f"RMSE: {rmse} > {expected_rmse}"

    max_abs_error = np.max(np.abs(diff))
    assert max_abs_error <= expected_max_error, (
        f"Max Error: {max_abs_error} > {expected_max_error}"
    )


with h5py.File(DATA_PATH / "versions.h5") as h5f:
    VERSION_TESTS = [
        f"{group}/{key}" for group in ["v0", "v1"] for key in h5f[group].keys()
    ]


@pytest.mark.decode_only
@pytest.mark.parametrize("h5path", VERSION_TESTS)
def test_versions(h5path: str):
    with h5py.File(DATA_PATH / "versions.h5") as h5f:
        ref_name = h5path.split("_", 1)[-1]
        if ref_name.startswith("be_"):
            ref_name = ref_name[3:]
        ref_entity = h5f["uncompressed"][ref_name]
        if isinstance(ref_entity, h5py.Group):
            ref_data = ref_entity["data"][()]
        else:
            ref_data = ref_entity[()]

        entity = h5f[h5path]
        if isinstance(entity, h5py.Group):
            dataset = entity["data"]
        else:
            dataset = entity
        decompressed_data = dataset[()]
        expected_rmse = dataset.attrs["RMSE"]
        expected_max_error = dataset.attrs["MAX_ABS_ERROR"]

        diff = decompressed_data.astype(np.float64) - ref_data.astype(np.float64)

        rmse = float(np.sqrt(np.mean(diff * diff)))
        rmse_tolerance = 0.01 * expected_rmse
        assert rmse <= expected_rmse + rmse_tolerance, (
            f"name: {h5path}, RMSE: {rmse} > {expected_rmse}"
        )

        max_abs_error = np.max(np.abs(diff))
        assert max_abs_error <= expected_max_error, (
            f"name: {h5path}, Max Error: {max_abs_error} > {expected_max_error}"
        )
