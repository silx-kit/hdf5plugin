
UNAME = $(shell uname)
ifeq (${UNAME}, Darwin)
  CXX = clang++
  CC = clang
  SPERR_INCDIR=/Users/shaomeng/Git/SPERR/install/include
  SPERR_LIBDIR=/Users/shaomeng/Git/SPERR/install/lib
else
  CXX = g++
  CC = gcc
  SPERR_INCDIR=/home/shaomeng/Git/SPERR/install/include
  SPERR_LIBDIR=/home/shaomeng/Git/SPERR/install/lib
endif

FLAGS = -g -O3

all: 2d.out 3d.out

2d.out: 2d.c
	${CC} ${FLAGS} -o $@ $^ -I${SPERR_INCDIR} -L${SPERR_LIBDIR} -Wl,-rpath,${SPERR_LIBDIR} -lSPERR

3d.out: 3d.c
	${CC} ${FLAGS} -o $@ $^ -I${SPERR_INCDIR} -L${SPERR_LIBDIR} -Wl,-rpath,${SPERR_LIBDIR} -lSPERR


clean:
	rm -f 2d.out 3d.out
