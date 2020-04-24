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

# This script is intended to be run by Hudson continuous integration
# system to build the FCICOMP software.
# 
# The FCICOMP_COTS_ROOT environment variable should be set before
# launching this script. It tells the location of the root of the COTS
# software.
#
# This script defines the location of the following list of COTS:
#   - ZLIB_ROOT
#   - HDF5_ROOT
#   - NETCDF_ROOT
#   - NETCDF_SOURCE_ROOT
#   - CHARLS_ROOT
#
# This also script defines the ${FCICOMP_ROOT} directory as the
# directory where this script is located. It should be the location of
# the root of the FCICOMP software.
# 
# It launches the build script ${FCICOMP_ROOT}/gen/build.sh in the
# "coverage" mode to compile the FCICOMP software in debug mode, run the
# unit test and check the coverage.
#
# Usage example:
# -------------
#  export FCICOMP_COTS_ROOT=/path/to/FCICOMP_COTS_ROOT
#  [ -d build ] && rm -rf build/
#  ./build.sh
#

set -o nounset
set -o errexit

# If the FCICOMP_COTS_ROOT environment variable is not set, 
# print an error message
if [[ -z ${FCICOMP_COTS_ROOT:-} ]]; then
    echo "Error: the FCICOMP_COTS_ROOT is not set."
    echo "Please set the FCICOMP_COTS_ROOT environment variable before launching this script."
    exit 1
fi

# If the FCICOMP_ROOT environment variable is not set, set the default
if [[ -z ${FCICOMP_ROOT:-} ]]; then
    FCICOMP_ROOT=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
fi

# Define the COTS location
export ZLIB_ROOT=${FCICOMP_COTS_ROOT}/zlib/V_1_2_8
export HDF5_ROOT=${FCICOMP_COTS_ROOT}/hdf5/V_1_8_14
export NETCDF_ROOT=${FCICOMP_COTS_ROOT}/netcdf/V_4_3_2
export NETCDF_SOURCE_ROOT=${FCICOMP_COTS_ROOT}/netcdf/V_4_3_2_SOURCE
export CHARLS_ROOT=${FCICOMP_COTS_ROOT}/charls/V_1_0


# Call the build_fcicomp.sh script of the ./build_fcidecomp.sh script
if [[ -f "${FCICOMP_ROOT}/build_fcicomp.sh" ]]; then
	./build_fcicomp.sh $@
	exit $?
elif [[ -f "${FCICOMP_ROOT}/build_fcidecomp.sh" ]]; then
	./build_fcidecomp.sh $@
	exit $?
else
	echo "Cannot find script build_fcicomp.sh or build_fcidecomp.sh "
	exit 1
fi
