.PHONY: all clean
ALL=example_sz example_lp
all: $(ALL)
clean:
	$(RM) $(ALL) $(wildcard *.o)
example_sz: CFLAGS=$(shell pkg-config --cflags sz)
example_sz: LDFLAGS=$(shell pkg-config --libs sz)
example_lp: CFLAGS=$(shell pkg-config --cflags libpressio)
example_lp: LDFLAGS=$(shell pkg-config --libs libpressio)

