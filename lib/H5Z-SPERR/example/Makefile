
UNAME = $(shell uname)
ifeq (${UNAME}, Darwin)
  CC = clang
else
  CC = gcc
endif

FLAGS = -g -O3

simple_xy_nc4.out: simple_xy_nc4_wr.c
	${CC} ${FLAGS} -o $@ $^ -lnetcdf -lm


clean:
	rm -f simple_xy_nc4.out simple_xy_nc4.nc
