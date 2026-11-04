#!/bin/bash


set -e


set -x


echo "Creating/cleaning build directory..."
rm -rf build
mkdir -p build

echo "Entering build directory..."
cd build


if [ -n "$CONDA_PREFIX" ]; then
    echo "Conda environment detected at: $CONDA_PREFIX"
    PYTHON_PATH="$CONDA_PREFIX/bin/python"
else
    echo "No Conda environment detected, using system Python"
    PYTHON_PATH=$(which python3)
fi


echo "Configuring project with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3" \
    -DPython_EXECUTABLE="$PYTHON_PATH" \
    -DPYTHON_EXECUTABLE="$PYTHON_PATH" \
    -DPython_ROOT_DIR="$CONDA_PREFIX" \
    -DCMAKE_PREFIX_PATH="$CONDA_PREFIX"


echo "Building project..."
make VERBOSE=1


cd ..


echo "Running tests..."
python3 py_tests/pyterse_test.py -v
python3 py_tests/filter_test.py -v


echo "Build and test process completed!"