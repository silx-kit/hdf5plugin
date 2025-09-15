# =============================================================
#
# Copyright 2021-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# =============================================================

# AUTHORS:
# - B-Open Solutions srl

"""
This test checks that JPEG-LS compressed MTG FCI L1C products are correctly decompressed using the
netCDF4 tool ``nccopy`` by comparing bands in the decompressed file with the same bands in
a reference output.
"""

import os
import subprocess

import netCDF4 as nc
import numpy as np
import pytest


TEST_DATA_PATH = os.environ.get("EPCT_TEST_DATA_DIR", "")
INPUT_PATH = os.path.join(TEST_DATA_PATH, "MTG", "MTGFCIL1")
COMP_FILEPATH = [os.path.join(INPUT_PATH, file_name) for file_name in (
    "W_XX-EUMETSAT-Darmstadt_IMG+SAT_MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY--"
    "DIS-NC4E_C_EUMT_20200405000845_GTT_DEV_20200405000330_20200405000345_N_JLS_T_0001_0015.nc",
    "W_XX-EUMETSAT-Darmstadt_IMG+SAT_MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY--"
    "DIS-NC4E_C_EUMT_20200405120015_GTT_DEV_20200405115500_20200405115515_N_JLS_T_0072_0021.nc"
)]
DECOMP_FILEPATH = [os.path.join(INPUT_PATH, file_name) for file_name in (
    "W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---"
    "NC4E_C_EUMT_20200405000845_GTT_DEV_20200405000330_20200405000345_N__T_0001_0015.nc",
    "W_XX-EUMETSAT-Darmstadt_IMG+SAT_MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---"
    "NC4E_C_EUMT_20200405120015_GTT_DEV_20200405115500_20200405115515_N__T_0072_0021.nc"
)]
BANDS = [
    "ir_105", "ir_123", "ir_133", "ir_38", "ir_87", "ir_97",
    "nir_13", "nir_16", "nir_22",
    "vis_04", "vis_05", "vis_06", "vis_08", "vis_09",
    "wv_63", "wv_73"
]
VARIABLES = [
    "effective_radiance", "pixel_quality", "index_map"
]


def test_decompression(tmpdir):

    assert "HDF5_PLUGIN_PATH" in os.environ.keys()

    for (comp_test_file, decomp_test_file) in zip(COMP_FILEPATH, DECOMP_FILEPATH):

        decomp_res_file = os.path.join(tmpdir, os.path.basename(decomp_test_file))
        process = subprocess.run(
            f"nccopy -F none {comp_test_file} {decomp_res_file}", shell=True
        )
        decomp_file_size = os.path.getsize(decomp_res_file)
        comp_file_size = os.path.getsize(comp_test_file)

        assert os.path.isfile(decomp_res_file)
        assert decomp_file_size > (comp_file_size * 4)

        ds_test = nc.Dataset(decomp_test_file, "r")
        ds_res = nc.Dataset(decomp_res_file, "r")

        for band in BANDS:
            for variable in VARIABLES:
                array_test = ds_test[f"data/{band}/measured/{variable}"][:]
                array_res = ds_res[f"data/{band}/measured/{variable}"][:]
                assert np.ma.allequal(array_test, array_res)


