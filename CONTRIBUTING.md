# Contributing to h5z-htj2k

## Setup

Install [`pixi`](https://pixi.sh) — it provisions the C/C++ toolchain, HDF5, and Python test dependencies, no manual setup needed:

```bash
curl -fsSL https://pixi.sh/install.sh | bash
```

From the root folder of this project, run: `pixi install --all --frozen`

## Build

- `pixi run build` — configure (CMake+Ninja) and build the filter.
- `pixi run build "-DDECODE_ONLY=ON"` — build with extra CMake options.
- `pixi run distclean` — remove the build folder.

## Formatting and linting


- `pixi run lint` — format C/C++ (`clang-format`) and Python (`ruff` format and check).
- `pixi run check` — same as `lint`, but read-only (what CI runs).

## Tests

- `pixi run test` — build and run the full test suite.
- `pixi run distclean && pixi run test-decode-only` — test a decode-only build.
- `pixi run test-py` — test the pure-Python reference filter (no compiled plugin).

Useful pytest flags (see [`tests/conftest.py`](tests/conftest.py)):

- `--use-python-filter` — test the pure-Python filter instead of the compiled one.
- `--decode-only` — skip tests needing an encode-enabled build.
