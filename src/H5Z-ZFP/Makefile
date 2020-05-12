include config.make

.PHONY: help all clean dist install

help:
	@echo ""
	@echo ""
	@echo ""
	@echo "             This is H5Z-ZFP version $(H5Z_ZFP_VERSINFO)."
	@echo "See http://h5z-zfp.readthedocs.io/en/latest/ file for more info."
	@echo ""
	@echo "Typical make command is..."
	@echo ""
	@echo "    make CC=<C-compiler> HDF5_HOME=<path> ZFP_HOME=<path> PREFIX=<path> all"
	@echo ""
	@echo "where <path> is a dir whose children are include/lib/bin subdirs."
	@echo "Standard make variables (e.g. CFLAGS, LD, etc.) can be set as usual."
	@echo "Optionally, add FC=<fortran-compiler> to include Fortran support and tests."
	@echo ""
	@echo "Available make targets are..."
	@echo "    all - build everything"
	@echo "    check - all + run tests"
	@echo "    install - install compiled components"
	@echo "    clean - clean away all derived targets"
	@echo "    dist - create distribution tarfile"

all:
	cd src; $(MAKE) $(MAKEVARS) $@

check: all install
	cd test; $(MAKE) $(MAKEVARS) $@

install:
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
