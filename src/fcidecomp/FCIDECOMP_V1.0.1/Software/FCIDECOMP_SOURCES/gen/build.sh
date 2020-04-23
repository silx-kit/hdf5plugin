#!/bin/bash
# $Id: build.sh 778 2016-03-09 07:56:29Z delaunay $
# =============================================================
# 
# PROJECT  : FCI_COMPRESSION 
# 
# AUTHOR   : THALES Services
# 
# Copyright 2015 EUMETSAT
# 
# =============================================================
# HISTORY :
# 
# VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
#
# END-HISTORY
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
    make || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "debug" ]]; then
    # Build in debug mode
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "memcheck" ]]; then
    # Build in debug mode with test enable and memory check
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DMEMORY_CHECK=ON ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure  -T memcheck || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "coverage" ]]; then
    # Build in debug mode with test enable and test coverage
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCOVERAGE_TESTING=ON ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make || { echo "Error building ${module}." 1>&2 ; exit 1; }
    ctest --output-on-failure || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
    ctest -T coverage || { echo "Error during the test of ${module}." 1>&2 ; exit 1; }
elif [[ "$mode" == "release" ]]; then
    # Build in release mode
    cmake $cmake_options -DCMAKE_BUILD_TYPE=Release ${FCICOMP_ROOT}/${module} \
	|| { echo "Error configuring ${module}." 1>&2 ; exit 1; }
    make || { echo "Error building ${module}." 1>&2 ; exit 1; }
else
    echo "${BASH_SOURCE[0]}: Unknown building mode: $mode." 2>&1
    rm -rf $BUILD_DIR
    exit 1
fi
