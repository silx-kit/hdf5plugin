# Include config.make only if we're not looking for help
ifeq ($(findstring help, $(strip $(MAKECMDGOALS))),)
    ifeq ($(findstring tools, $(strip $(MAKECMDGOALS))),)
        ifeq ($(findstring clean, $(strip $(MAKECMDGOALS))),)
            ifeq ($(findstring dist, $(strip $(MAKECMDGOALS))),)
                include config.make
            endif
        endif
    endif
endif

# Get version string from H5Z-ZFP
H5Z_ZFP_VERSINFO := $(shell grep '^\#define H5Z_FILTER_ZFP_VERSION_[MP]' src/H5Zzfp_version.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3 2>/dev/null)

.PHONY: help all clean dist install

help:
	@echo ""
	@echo ""
	@echo ""
	@echo "                 This is H5Z-ZFP version $(H5Z_ZFP_VERSINFO)."
	@echo "See http://h5z-zfp.readthedocs.io/en/latest/ file for more info."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make CC=<C-compiler> HDF5_HOME=<path> ZFP_HOME=<path> PREFIX=<path> all"
	@echo ""
	@echo "where <path> is a dir whose children are include/lib/bin subdirs."
	@echo "HDF5_HOME can also be set using an INC,LIB,BIN triplet specifying"
	@echo "HDF5 include, library and binary dirs separated by commas."
	@echo "Standard make variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo "Optionally, add FC=<fortran-compiler> to include Fortran support and tests."
	@echo ""
	@echo "Available make targets are..."
	@echo "    all     - build everything needed for H5Z-ZFP plugin/lib"
	@echo "    check   - all + run tests"
	@echo "    tools   - build tools (currently just print_h5repack_farg)"
	@echo "    install - install plugin/lib"
	@echo "    clean   - clean away all derived targets"
	@echo "    dist    - create distribution tarfile"
	@echo "    help    - this help message"

all:
	cd src; $(MAKE) $(MAKEVARS) $@

check: all
	cd test; $(MAKE) $(MAKEVARS) $@

tools:
	cd test; $(MAKE) $(MAKEVARS) print_h5repack_farg

install: all
	cd src; $(MAKE) $(MAKEVARS) $@

clean:
	rm -f H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz
	cd src; $(MAKE) $(MAKEVARS) $@
	cd test; $(MAKE) $(MAKEVARS) $@

dist:	clean
	rm -rf H5Z-ZFP-$(H5Z_ZFP_VERSINFO) H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz; \
	mkdir H5Z-ZFP-$(H5Z_ZFP_VERSINFO); \
	tar cf - --exclude ".git*" --exclude H5Z-ZFP-$(H5Z_ZFP_VERSINFO) . | tar xf - -C H5Z-ZFP-$(H5Z_ZFP_VERSINFO); \
	tar cvf - H5Z-ZFP-$(H5Z_ZFP_VERSINFO) | gzip --best > H5Z-ZFP-$(H5Z_ZFP_VERSINFO).tar.gz; \
	rm -rf H5Z-ZFP-$(H5Z_ZFP_VERSINFO);
