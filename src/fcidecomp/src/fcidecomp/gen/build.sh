#!/bin/bash
# =============================================================
#
# Copyright 2015-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
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

# Script to build a module of the FCICOMP software.
# 
# Usage: ./build.sh [module] [mode] [cmake-options]
#   with module in {fcicomp-jpegls, fcicomp-necdf-ext, fcicomp-test, ...}
#   mode in: {release, debug, test, coverage, memcheck}
#   and cmake-options are passed to the cmake command line.
#
# The FCICOMP_ROOT environment variable should be set before launching
# this script. It tells the location of the root of the FCICOMP
# software.  It should be point on the upper directory of this script:
# gen/..
#
# This script launches cmake using the CMakeList.txt script at the in
# the module directory. The software is build in ${FCICOMP_ROOT}/build
# directory. Then it launches the build calling "make" and optionally
# run the unit tests calling "ctest".
#
# Example:
#  ./build.sh fcicomp-jpegls release \
#     -DCMAKE_PREFIX_PATH=/data/TEC_COMMON/external/charls/V_1_0 \
#     -DCMAKE_INSTALL_PREFIX=/usr/local
#

# AUTHORS:
# - THALES Services
# - B-Open Solutions srl

set -o nounset
set -o errexit

# Get the module to build
module=$1

# Get the building mode
mode=$2

# Get cmake options
shift 2
cmake_options=$@

# If the FCICOMP_ROOT environment variable is not set, set the default
# one: the upper directory of this script
if [[ -z ${FCICOMP_ROOT:-} ]]; then
    FCICOMP_ROOT=$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )
fi

# Define the build directory
BUILD_DIR=${FCICOMP_ROOT}/build/${module}

# Test only
if [[ "$mode" == "test" ]] && [[ -d $BUILD_DIR ]]; then
    # If the building directory already exists
    # Do not build but run the unit tests 
    cd $BUILD_DIR
    ctest --output-on-failure
    exit $?
fi

# Check that the building folder does not already exists
[[ -e $BUILD_DIR ]] && { echo "Remove the $BUILD_DIR folder first!"; exit 1; }
# Create the building folder and move into it
mkdir -p $BUILD_DIR || { echo "Error: cannot create the building directory: ${BUILD_DIR}." 1>&2 ; exit 1; }
cd $BUILD_DIR

# Message
echo "Building $module ..."

if [[ "$mode" == "test" ]]; then
    # Build in release mode with tests enable
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make VERBOSE=1 || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "debug" ]]; then
    # Build in debug mode
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make VERBOSE=1 || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "memcheck" ]]; then
    # Build in debug mode with test enable and memory check
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DMEMORY_CHECK=ON ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make VERBOSE=1 || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure  -T memcheck || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "coverage" ]]; then
    # Build in debug mode with test enable and test coverage
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCOVERAGE_TESTING=ON ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make VERBOSE=1 || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
    ctest -T coverage || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "release" ]]; then
    # Build in release mode
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Release ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make VERBOSE=1 || { echo "Error building ${module}." 1>&2 ; exit 1; }
else
    echo "${BASH_SOURCE[0]}: Unknown building mode: $mode." 2>&1
    rm -rf $BUILD_DIR
    exit 1
fi
