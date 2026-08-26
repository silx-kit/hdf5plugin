from pathlib import Path

import imagecodecs
import numpy as np
import pooch
import pytest
from PIL import Image
from utils import assert_decompress

OPENJPEG_DATA_COMMIT = "39524bd3a601d90ed8e0177559400d23945f96a9"
OPENJPEG_DATA_URL = (
    f"https://github.com/uclouvain/openjpeg-data/archive/{OPENJPEG_DATA_COMMIT}.tar.gz"
)


@pytest.fixture
def openjpeg_data_registry(cache):
    return pooch.create(
        path=cache.mkdir("openjpeg_data"),
        base_url=f"https://raw.githubusercontent.com/uclouvain/openjpeg-data/{OPENJPEG_DATA_COMMIT}/",
        registry={
            "input/nonregression/byte.tif": "sha256:e0fad3830408e34fa815d3663eac888595250671616accc28e6e55d1aca6c2f4",
            "input/nonregression/htj2k/byte_causal.jhc": "sha256:8d4a48d1cfff47420203283f3eb1b5c48f92dd1a6c23916a18934a44f154002f",
            "input/nonregression/Bretagne1.ppm": "sha256:66a8f6969dbd7302015a5d6945becf8a47948665e77c3e07b8f956638b190a7a",
            "input/nonregression/htj2k/Bretagne1_ht.j2k": "sha256:fa82708024118771d58d90231e406ec40ad3eedf623927697c3659c123d89ec4",
            "input/nonregression/htj2k/Bretagne1_ht_lossy.j2k": "sha256:372c5b8c5f136c8cabca266d1af7bd8078c0fee14441e7dc850f228d28152d2d",
        },
    )


def test_byte(openjpeg_data_registry):
    codestream = Path(
        openjpeg_data_registry.fetch("input/nonregression/htj2k/byte_causal.jhc")
    ).read_bytes()
    expected = imagecodecs.imread(
        openjpeg_data_registry.fetch("input/nonregression/byte.tif")
    )
    assert_decompress(codestream, expected)


def test_bretagne1(openjpeg_data_registry):
    codestream = Path(
        openjpeg_data_registry.fetch("input/nonregression/htj2k/Bretagne1_ht.j2k")
    ).read_bytes()
    expected = np.array(
        Image.open(openjpeg_data_registry.fetch("input/nonregression/Bretagne1.ppm"))
    )
    assert_decompress(codestream, expected)


def test_bretagne1_lossy(openjpeg_data_registry):
    codestream = Path(
        openjpeg_data_registry.fetch("input/nonregression/htj2k/Bretagne1_ht_lossy.j2k")
    ).read_bytes()
    expected = np.array(
        Image.open(openjpeg_data_registry.fetch("input/nonregression/Bretagne1.ppm"))
    )
    assert_decompress(codestream, expected, atol=255, rmse_tolerance=6.85)
