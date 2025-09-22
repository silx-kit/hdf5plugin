#!/bin/bash

if [ $# -ne 3 ]; 
then
    echo "Usage: ./a.out input_filename dim_x dim_y"
    exit
fi

Sam_output=./tmp/sam.double
Qcc_output=./tmp/qcc.double

./test_sam_dwt2d.out "$1" "$2" "$3" "$Sam_output"
./test_qcc_dwt2d.out "$1" "$2" "$3" "$Qcc_output"
./compare_raw.out "$Sam_output" "$Qcc_output"
