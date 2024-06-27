#!/bin/bash

bpp=(0.125 0.25 0.5 1 2 4)
build_dir_name=build-gcc-9.3

for t in ${bpp[*]}; do
    ../$build_dir_name/compressor_3d ../test_data/wmag128.float 128 128 128 XYZ tmp $t >> 128_cube.result
done

for t in ${bpp[*]}; do
    ../$build_dir_name/compressor_3d ../test_data/wmag256.float 256 256 256 XYZ tmp $t >> 256_cube.result
done

for t in ${bpp[*]}; do
    ../$build_dir_name/compressor_3d ../test_data/wmag512.float 512 512 512 XYZ tmp $t >> 512_cube.result
done


../$build_dir_name/compressor_3d ../test_data/wmag128.float 128 128 128 XYZ 128.bitstream 4.1
../$build_dir_name/compressor_3d ../test_data/wmag256.float 256 256 256 XYZ 256.bitstream 4.1
../$build_dir_name/compressor_3d ../test_data/wmag512.float 512 512 512 XYZ 512.bitstream 4.1


for t in ${bpp[*]}; do
    ../$build_dir_name/decompressor_3d ./128.bitstream $t tmp >> 128_cube.result
done

for t in ${bpp[*]}; do
    ../$build_dir_name/decompressor_3d ./256.bitstream $t tmp >> 256_cube.result
done

for t in ${bpp[*]}; do
    ../$build_dir_name/decompressor_3d ./512.bitstream $t tmp >> 512_cube.result
done


rm -f 128.bitstream 256.bitstream 512.bitstream tmp
