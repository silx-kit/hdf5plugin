include ../config.make

.PHONY: all lib clean

# The filter compiled to be used as a plugin
H5Zzfp_plugin.o: H5Zzfp.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

# The filter compiled to be used as a library
H5Zzfp_lib.o: H5Zzfp.c
	$(CC) -c $< -o $@ -DH5Z_ZFP_AS_LIB $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

# The plugin shared lib (HDF5 expects it to be named "lib*.{so,dylib}"
plugin/libh5zzfp.$(SOEXT): H5Zzfp_plugin.o
	rm -rf plugin
	mkdir plugin
	$(CC) $< $(SHFLAG) -o plugin/libh5zzfp.$(SOEXT) \
	    $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) \
	    -L$(ZFP_LIB) -L$(HDF5_LIB) -lhdf5 -lzfp $(LDFLAGS)

# Alias target for filter plugin
plugin: plugin/libh5zzfp.$(SOEXT)

# C language properties interface
H5Zzfp_props.o: H5Zzfp_props.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

# Fortran language properties interface
H5Zzfp_props_f.o H5Zzfp_props_f.mod: H5Zzfp_props_f.F90
	$(FC) -c $< -o $@ $(FCFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

# The filter member of the filter library
libh5zzfp.a(H5Zzfp_lib.o): H5Zzfp_lib.o
	$(AR) cr libh5zzfp.a $<

# The C properties interface member of the filter library
libh5zzfp.a(H5Zzfp_props.o): H5Zzfp_props.o
	$(AR) cr libh5zzfp.a $<

# The Fortran properties interface member of the filter library
libh5zzfp.a(H5Zzfp_props_f.o): H5Zzfp_props_f.o
	$(AR) cr libh5zzfp.a $<

# Alias target for filter library, conditionally includes Fortran
LIBOBJ = libh5zzfp.a(H5Zzfp_lib.o) libh5zzfp.a(H5Zzfp_props.o)
ifneq ($(FC),)
LIBOBJ += libh5zzfp.a(H5Zzfp_props_f.o)
endif
lib: $(LIBOBJ)

all: lib plugin

install: all
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/{plugin,include,lib}
	$(INSTALL) plugin/libh5zzfp.$(SOEXT) $(DESTDIR)$(PREFIX)/plugin
	$(INSTALL) libh5zzfp.a $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 644 H5Zzfp.h H5Zzfp_lib.h H5Zzfp_plugin.h H5Zzfp_props.h $(DESTDIR)$(PREFIX)/include
ifneq ($(FC),)
	$(INSTALL) -m 644 *.[mM][oO][dD] $(DESTDIR)$(PREFIX)/include
endif

clean:
	rm -rf plugin
	rm -f libh5zzfp.a *.o *.[mM][oO][dD]
	rm -f *.gcno *.gcda *.gcov
	rm -f H5Zzfp_props_f.mod
