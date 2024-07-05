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

# AUTHORS:
# - THALES Services
# - B-Open Solutions srl

# Script to install a module of the FCICOMP software.
# 
# Usage: ./install.sh [module]
#   with module in {fcicomp-jpegls, fcicomp-necdf-ext, fcicomp-test, ...}
#
# The FCICOMP_ROOT environment variable should be set before launching
# this script. It tells the location of the root of the FCICOMP
# software.  It should be point on the upper directory of this script:
# gen/..
#
# This script should be called after the ${FCICOMP_ROOT}/gen/build.sh
# script used to build the module.
#  
# This script launches "make install" in the module build directory:
# ${FCICOMP_ROOT}/build/module.
#
# It also copies the install_manifest.txt file in the install directory.
#
# Example:
#  ./install.sh fcicomp-jpegls
#

set -o nounset
set -o errexit

# define programs used
AWK="awk"

# Get the module to build
MODULE=$1

# If the FCICOMP_ROOT environment variable is not set, set the default
# one: the upper directory of this script
if [[ -z ${FCICOMP_ROOT:-} ]]; then
    FCICOMP_ROOT=$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )
fi

# Define the build directory
BUILD_DIR=${FCICOMP_ROOT}/build/${MODULE}

# Move to the build directory
cd $BUILD_DIR

# ====================================================================
# This function reads the input file and returns the value
# corresponding to the input key
# ====================================================================
function get_key_value()
{
    local key=$1
    local file=$2

    # Get the line corresponding to that key in input file
    # and return an error if the line does not exist
    local line=$(cat $file | $AWK -v key="$key"  "/^$key[ ]*=.*$/") || { echo "Error: the required field \"$key\" does not exist in the file: $file"; exit 1; }
    # Extract the value at the right side of the "="
    local value=$(echo $line | $AWK -F "=" '{print $2}') || { echo "Error: cannot read the value of \"$key\" in the file: $file"; exit 1; }
    # Check that the value is not empty
    [[ "x$value" == "x" ]] && { echo "Error: cannot read the value of \"$key\" in the file: $file"; exit 1; }
    # Return the value
    echo $value
}


# ====================================================================
# Try to read the install_prefix in the CMakeCahe.txt file
# ====================================================================
function get_install_prefix()
{
    # output variable
    local __install_prefix=$1
    
    # Define the CMakeCahe.txt file
    cmakecahe_file=${BUILD_DIR}/CMakeCache.txt
    if [[ -f ${cmakecahe_file} ]]; then
	# Locate the CMAKE_INSTALL_PREFIX in the CMakeCache.txt file
	install_prefix=$(get_key_value "CMAKE_INSTALL_PREFIX:PATH" $cmakecahe_file)
	# Return the install_prefix
	eval $__install_prefix="$install_prefix"
    else
	echo "Error: Cannot find file: ${cmakecahe_file}."
    fi
}

# ====================================================================
# Copy the install_manifest.txt file 
# from the building directory to the install directory
# ====================================================================
function copy_install_manifest()
{
    # Define the install_manifest.txt file
    install_manifest_file=${BUILD_DIR}/install_manifest.txt
    if [[ -f ${install_manifest_file} ]]; then
        # Try to read the install_prefix in the CMakeCahe.txt file
	get_install_prefix install_prefix
	if ! [[ -z ${install_prefix:-} ]]; then
	    
	    # Define the destination file and directory
	    dest_dir=${install_prefix}/share/cmake
	    dest=${dest_dir}/${MODULE}_install_manifest.txt
	    # Create the destination directory if it does not exist
	    [[ ! -e $dest_dir ]] && { echo "Creating directory $dest_dir"; mkdir -p $dest_dir; }

	    # Append one line in the install_manifest.txt file
	    echo $dest >> ${install_manifest_file}
	    
	    # Copy the install_manifest.txt file
	    echo "-- Copying: ${install_manifest_file} to ${dest}"
	    cp  ${install_manifest_file} ${dest}
	else
	    echo "Warning: Cannot copy the install_manifest.txt file to the install directory: Install directory is not known."
	fi
    else
	echo "Warning: The file install_manifest.txt has not been found in the building directory ${BUILD_DIR}."
    fi
}


# ====================================================================
# Main function
# ====================================================================
function main()
{
    # Message
    echo
    echo "Installing $MODULE ..."

    # Perform the install
    make install VERBOSE=1 || { echo "Error: cannot install ${MODULE}." 1>&2 ; exit 1; }

    # Copy the install_manifest.txt file from the building directory to the install directory
    copy_install_manifest
}

main $@