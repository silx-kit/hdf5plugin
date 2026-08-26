#!/usr/bin/env bash
# Build a self-contained s390x wheel of imagecodecs (imcd, shared, htj2k,
# tiff extensions only). vendored/OpenJPH is built and installed under
# /usr/local so imagecodecs can link against it, then `auditwheel repair`
# bundles libopenjph (and any other non-system shared libs) into the wheel
# itself, so it doesn't rely on /usr/local being populated wherever the
# wheel is later installed.
#
# Intended to run *inside* an s390x Debian trixie environment, e.g.:
#
#   docker run --rm -v "$PWD":/work -w /work --platform linux/s390x \
#     debian:trixie ./scripts/build-imagecodecs-wheel-s390x.sh
#
# (requires qemu-user-static / binfmt support for cross-arch emulation).
#
# The resulting repaired wheel is written to OUTPUT_DIR (default:
# ./wheelhouse). Set AUDITWHEEL_PLAT to force a specific auditwheel
# platform tag (e.g. manylinux_2_28_s390x) instead of letting auditwheel
# auto-detect one.

set -euo pipefail

OUTPUT_DIR="${OUTPUT_DIR:-$PWD/wheelhouse}"
OPENJPH_SRC="${OPENJPH_SRC:-$PWD/vendored/OpenJPH}"
AUDITWHEEL_PLAT="${AUDITWHEEL_PLAT:-}"

TMP_DIRS=()
cleanup() { rm -rf "${TMP_DIRS[@]}"; }
trap cleanup EXIT

OPENJPH_BUILD_DIR="$(mktemp -d)"; TMP_DIRS+=("$OPENJPH_BUILD_DIR")
IC_SHIM_DIR="$(mktemp -d)"; TMP_DIRS+=("$IC_SHIM_DIR")
RAW_WHEEL_DIR="$(mktemp -d)"; TMP_DIRS+=("$RAW_WHEEL_DIR")

apt-get update -q -y
apt_packages=(
  build-essential pkg-config ca-certificates cmake ninja-build patchelf
  python3-dev python3-pip python3-setuptools python3-wheel
  python3-numpy libhdf5-dev libtiff-dev
)
apt-get install -q -y --no-install-recommends "${apt_packages[@]}"

# Debian's libopenjph-dev (0.21) is too old for imagecodecs
openjph_cmake_args=(
  -G Ninja -B "$OPENJPH_BUILD_DIR" -S "$OPENJPH_SRC"
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
  -DOJPH_ENABLE_TIFF_SUPPORT=OFF -DOJPH_BUILD_TESTS=OFF
  -DOJPH_BUILD_EXECUTABLES=OFF -DOJPH_BUILD_STREAM_EXPAND=OFF
  -DOJPH_BUILD_FUZZER=OFF
)
cmake "${openjph_cmake_args[@]}"
cmake --build "$OPENJPH_BUILD_DIR"
cmake --install "$OPENJPH_BUILD_DIR"
ldconfig

# imagecodecs setup.py unconditionally touches a couple of extension keys
# it assumes are always present, even when IMAGECODECS_ONLY drops them, so
# stub out its distributor hook to skip that logic.
cat > "$IC_SHIM_DIR/imagecodecs_distributor_setup.py" <<'PYEOF'
def customize_build(extensions, options):
    options.pop("shared_utility_qualified_name", None)
PYEOF

mkdir -p "$OUTPUT_DIR"
export PIP_BREAK_SYSTEM_PACKAGES=1
export IMAGECODECS_ONLY=imcd,shared,htj2k,tiff
export PYTHONPATH="$IC_SHIM_DIR"
export CPATH=/usr/local/include
export LIBRARY_PATH=/usr/local/lib
pip3 install "cython>=3.1" auditwheel
pip3_wheel_args=(
  --no-build-isolation --no-deps --no-binary imagecodecs
  --wheel-dir "$RAW_WHEEL_DIR" imagecodecs
)
pip3 wheel "${pip3_wheel_args[@]}"

auditwheel_args=(repair -w "$OUTPUT_DIR")
if [[ -n "$AUDITWHEEL_PLAT" ]]; then
  auditwheel_args+=(--plat "$AUDITWHEEL_PLAT")
fi
python3 -m auditwheel "${auditwheel_args[@]}" "$RAW_WHEEL_DIR"/imagecodecs-*.whl

echo "Repaired wheel built in $OUTPUT_DIR:"
ls -l "$OUTPUT_DIR"
