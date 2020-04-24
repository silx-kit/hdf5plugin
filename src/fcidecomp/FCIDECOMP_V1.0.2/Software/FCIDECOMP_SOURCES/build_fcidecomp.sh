#!/bin/bash
# $Id: build_fcidecomp.sh 778 2016-03-09 07:56:29Z delaunay $
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
# system to build the FCIDECOMP software.
# 
# The following environment variable should be set before
# launching this script. It tells the location of the COTS software.
#
#   - HDF5_ROOT
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
#  [ -d build ] && rm -rf build/
#  ./build_fcidecomp.sh
#

set -o nounset
set -o errexit

function usage()
{
    echo "Usage: $0 mode module"
    echo 
    echo "mode    Building mode in {release, debug, test, coverage, memcheck}."
    echo "module  Module to build in {fcidecomp, fcicomp-jpegls, fcicomp-H5Zjpegls}."
    exit 2
}

function main()
{

if [[ ${1:-} == "-h" ]] || [[ ${1:-} == "-u" ]]; then 
    # Print the usage
    usage
fi

# Get the name of the modules the user want to build
if [[ $# == 0 ]]; then 
    # Compile in debug mode and perform unit test coverage 
    build_mode="coverage" 
    module="all"
elif [[ $# == 1 ]]; then 
    build_mode=$1
    module="all"
else
    build_mode=$1
    module=$2
fi

# Check that HDF5_ROOT is set
if [[ -z ${HDF5_ROOT:-} ]]; then
    echo "Error: the HDF5_ROOT is not set."
    echo "Please set the HDF5_ROOT environment variable before launching this script."
    exit 1
fi
# Check that HDF5_ROOT point on a directory
if [[ ! -d  ${HDF5_ROOT:-} ]]; then
    echo "Error: HDF5 directory cannot be reached:  ${HDF5_ROOT:-}"
    echo "Please set the HDF5_ROOT environment variable pointing on a valid directory."
    exit 1
fi
# Check that CHARLS_ROOT is set
if [[ -z ${CHARLS_ROOT:-} ]]; then
    echo "Error: the CHARLS_ROOT is not set."
    echo "Please set the CHARLS_ROOT environment variable before launching this script."
    exit 1
fi
# Check that CHARLS_ROOT point on a directory
if [[ ! -d ${CHARLS_ROOT:-} ]]; then
    echo "Error: CHARLS directory cannot be reached: ${CHARLS_ROOT:-}"
    echo "Please set the CHARLS_ROOT environment variable pointing on a valid directory."
    exit 1
fi

# If the FCICOMP_ROOT environment variable is not set, set the default
# one: this script directory
if [[ -z ${FCICOMP_ROOT:-} ]]; then
    FCICOMP_ROOT=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
fi

# Define an install path
FCICOMP_INSTALL_PATH=${FCICOMP_ROOT}/build/local
[[ ! -d $FCICOMP_INSTALL_PATH ]] && mkdir -p $FCICOMP_INSTALL_PATH

# Define the location of the building script 
BUILD_SCRIPT=${FCICOMP_ROOT}/gen/build.sh
# Define the location of the install script
INSTALL_SCRIPT=${FCICOMP_ROOT}/gen/install.sh

# --------------------------------------------------------------------
# Build FCICOMP-JPEGLS
# --------------------------------------------------------------------

PACKAGE="fcicomp-jpegls"

if [[ x${module}x == x${PACKAGE}x ]] || [[ x${module}x == x"all"x ]]; then

    echo 
    echo " ===============  FCICOMP-JPEGLS ==============================="
    echo 

    # Define where to find CharLS and where to install fcicomp-jpegls
    BUILD_OPTIONS="-DCMAKE_PREFIX_PATH=${CHARLS_ROOT} -DCMAKE_INSTALL_PREFIX=${FCICOMP_INSTALL_PATH}"
    # Create the command line
    build_cmd="${BUILD_SCRIPT} ${PACKAGE} ${build_mode} ${BUILD_OPTIONS}"
    install_cmd="${INSTALL_SCRIPT} ${PACKAGE} "
    # Launch the commands
    $build_cmd || { echo "Error: cannot build ${PACKAGE}." 1>&2 ; exit 1; }
    $install_cmd || { echo "Error: cannot install ${PACKAGE}." 1>&2 ; exit 1; }
    
fi

# Define where is installed fcicomp-jpegls
FCICOMP_JPEGLS_ROOT=${FCICOMP_INSTALL_PATH}

# --------------------------------------------------------------------
# Build FCICOMP-H5ZJPEGLS
# --------------------------------------------------------------------

PACKAGE="fcicomp-H5Zjpegls"

if [[ x${module}x == x${PACKAGE}x ]] || [[ x${module}x == x"all"x ]]; then

    echo 
    echo " ===============  FCICOMP-H5ZJPEGLS ==================="
    echo 

    # Define where to find CharLS, Zlib and HDF5 and where to install the filter
    BUILD_OPTIONS="-DCMAKE_PREFIX_PATH=${FCICOMP_JPEGLS_ROOT};${HDF5_ROOT} -DCMAKE_INSTALL_PREFIX=${FCICOMP_INSTALL_PATH}"
    # Create the command line
    build_cmd="${BUILD_SCRIPT} ${PACKAGE} ${build_mode} ${BUILD_OPTIONS}"
    install_cmd="${INSTALL_SCRIPT} ${PACKAGE} "
    # Launch the commands
    $build_cmd || { echo "Error: cannot build ${PACKAGE}." 1>&2 ; exit 1; }
    $install_cmd || { echo "Error: cannot install ${PACKAGE}." 1>&2 ; exit 1; }
    
fi

}

# Call the main function
main $@