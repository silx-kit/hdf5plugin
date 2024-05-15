#!/bin/bash

# create a few symbolic links
FILE=bin
if test -e "$FILE"; then
  echo "--> C++ bin dir exists"
else
  ln -s ../../install/bin .
fi

FILE=test_data
if test -e "$FILE"; then
  echo "--> test data dir exists"
else
  ln -s ../../test_data .
fi

# generate C executables
make 2d.out 3d.out

CSTREAM=output.stream
CDECOMP=output.data
CPPSTREAM=cpp.stream
CPPDECOMP=cpp.data

#
# test 2D BPP
#
FILE=./test_data/lena512.float 
Q=2.5
./2d.out $FILE 512 512 1 $Q
./bin/sperr2d -c --ftype 32 --dims 512 512 --o_bitstream $CPPSTREAM --o_decomp_f $CPPDECOMP --bpp $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 2D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 2D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP

#
# test 2D PSNR
#
FILE=./test_data/999x999.float 
Q=90.0
./2d.out $FILE 999 999 2 $Q
./bin/sperr2d -c --ftype 32 --dims 999 999 --o_bitstream $CPPSTREAM --o_decomp_f $CPPDECOMP --psnr $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 2D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 2D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP


#
# test 2D PWE
#
FILE=./test_data/vorticity.512_512
Q=1e-8
./2d.out $FILE 512 512 3 $Q
./bin/sperr2d -c --ftype 32 --dims 512 512 --o_bitstream $CPPSTREAM --o_decomp_f $CPPDECOMP --pwe $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 2D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 2D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP


#
# test 3D BPP
#
FILE=./test_data/density_128x128x256.d64
Q=2.6
./3d.out $FILE 128 128 256 1 $Q -d
./bin/sperr3d -c --ftype 64 --dims 128 128 256 --o_bitstream $CPPSTREAM --o_decomp_f64 $CPPDECOMP --bpp $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 3D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 3D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP


#
# test 3D PSNR
#
FILE=./test_data/density_128x128x256.d64
Q=102.5
./3d.out $FILE 128 128 256 2 $Q -d
./bin/sperr3d -c --ftype 64 --dims 128 128 256 --o_bitstream $CPPSTREAM --o_decomp_f64 $CPPDECOMP --psnr $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 3D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 3D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP


#
# test 3D PWE
#
FILE=./test_data/density_128x128x256.d64
Q=4e-5
./3d.out $FILE 128 128 256 3 $Q -d
./bin/sperr3d -c --ftype 64 --dims 128 128 256 --o_bitstream $CPPSTREAM --o_decomp_f64 $CPPDECOMP --pwe $Q $FILE

if diff $CSTREAM $CPPSTREAM; then
  echo "--> C and C++ utilities produce the same bitstream on 3D test data $FILE"
else
  echo "--> C and C++ utilities produce different bitstream on 3D test data $FILE"
  exit 1
fi
if diff $CDECOMP $CPPDECOMP; then
  echo "--> C and C++ utilities produce the same decompressed file on $FILE"
else
  echo "--> C and C++ utilities produce different decompressed file on $FILE"
  exit 1
fi
rm -f $CSTREAM $CPPSTREAM $CDECOMP $CPPDECOMP

