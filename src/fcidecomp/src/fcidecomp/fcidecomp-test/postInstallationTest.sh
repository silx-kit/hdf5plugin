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

set -o nounset
set -o errexit


# Perform FCI decompression software post-installation tests.
#
# Usage:
# 
# $ ./postInstallationTest.sh
#   

NC_DUMP="ncdump"

# Get the path to that script
SCRIPT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# Define the data directory
DATA_DIR=${SCRIPT_PATH}/data

# Define the end user simulator input
NC_FILE=${DATA_DIR}/sample.nc

# Define the ncdump outputs
NC_DUMP_OUT_DIR=${DATA_DIR}
OUTPUT_FILE=${DATA_DIR}/sample.txt

# Define the reference data files
SAMPLE_REF=${DATA_DIR}/sample_ref.txt

# ====================================================================
# Print the usage
# ====================================================================
function usage {
    echo "Usage: $0" 1>&2
    exit 1
}

# ====================================================================
# Print the help
# ====================================================================
function help { 
    echo "Check that the installation of the FCIDECOMP software is correct."
    echo ""
    echo "This script uses ncdump utility to read the data inside the"
    echo " compressed netCDF file and checks that the data are correct"
    echo " by comparing the extracted data with the reference data."
    echo ""
    usage $@
}

# ====================================================================
# Return 0 if the command exist
# ====================================================================
function command_exists () {
    type "$1" &> /dev/null ;
}

# ====================================================================
# Exit failure function
# ====================================================================
function exit_failure {
    echo "*** FAIL: Post-installation test failed! ***"
    exit 1;
}

# ====================================================================
# Parse the input arguments
# ====================================================================
function parse_inputs {

    # check the input number of arguments
    # otherwise print the usage
    if [[ $# -gt 1 ]]; then
	usage
	return 0
    fi

    if [[ $# -ge 1 ]]; then
        # check the first argument
	case ${1:-} in
	    -h|--help)
		help
		;;
	    *)  # default
		usage
		;;
	esac
    fi   
}

# ====================================================================
# Main
# ====================================================================
function main {
    # Parse the input command line
    parse_inputs $@

    # Check that ncdump program exists
    ! command_exists $NC_DUMP && { echo "Error: $NC_DUMP cannot be run. Check your installation of netCDF and set ncdump utility in your PATH environment variable." ; exit_failure ; }
        
    # Create the commmand line for the enduser simulator
    echo "$NC_DUMP: Reading file: $NC_FILE ..."
    cmd="${NC_DUMP} ${NC_FILE}"
    $cmd > $OUTPUT_FILE || { echo "${NC_DUMP}: Error reading file: $NC_FILE" ; exit_failure ; }

    echo "Comparing the output files to the reference files:"
    echo "  Comparing $OUTPUT_FILE"
    echo "         to $SAMPLE_REF"
    # Compare the output radiance data file to the reference file
    diff -q $OUTPUT_FILE $SAMPLE_REF || { 
	echo "Error: the output file does not match the reference file!";
	echo $OUTPUT_FILE
	echo $SAMPLE_REF
	exit_failure ; }
	
    echo "*** SUCCESS! ***"
    return 0
}

# Launch the main function
main $@



