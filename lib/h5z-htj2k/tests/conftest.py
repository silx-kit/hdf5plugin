import htj2k_filter
import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--use-python-filter",
        action="store_true",
        help="Run tests with the Python implementation of the filter",
    )
    parser.addoption(
        "--decode-only",
        action="store_true",
        help="Only run decompression tests, skipping compression and roundtrip tests",
    )


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "decode_only: test exercises only the decompression (decode) path"
    )

    if config.getoption("--use-python-filter"):
        if not htj2k_filter.register(force=True):
            raise RuntimeError("Failed to register Python filter")


def pytest_collection_modifyitems(config, items):
    if not config.getoption("--decode-only"):
        return

    skip_not_decode = pytest.mark.skip(
        reason="--decode-only: skips tests requiring encode enabled filter"
    )
    for item in items:
        if not item.get_closest_marker("decode_only"):
            item.add_marker(skip_not_decode)
