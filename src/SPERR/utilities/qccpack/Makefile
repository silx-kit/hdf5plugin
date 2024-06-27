
CC=gcc
CXX=g++
FLAGS=-g -O2 -Wall
QCC_DIR=/home/shaomeng/Git/QccPack-0.63-1/install

ARCH=$(shell uname)
ifeq ($(ARCH), Darwin)
  CC=clang
  CXX=clang++
  CFLAGS=-g -Wall -Wno-unused-variable
  CXXFLAGS=-std=c++14 -g -Wall -Wno-unused-private-field -Wno-unused-variable
endif


dwt3d.out: example_qcc_dwt3d.c
	${CC} ${FLAGS} -o $@ $^ -I${QCC_DIR}/include -L${QCC_DIR}/lib -Wl,-rpath,${QCC_DIR}/lib -lQccPack -lm

clean:
	rm -rf *.out* *.o *.tmp
