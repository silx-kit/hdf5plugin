include ../config.make

NO_COLOR=\033[0m
OK_COLOR=\033[32;01m
ERROR_COLOR=\033[31;01m
SKIP_COLOR=\033[0;34m
padlimit=60
padlength=60

ZFP_LIBS = -lzfp
ifeq ($(ZFP_HAS_CFP),1)
    ZFP_LIBS += -lcfp -lstdc++
endif

.PHONY: lib plugin check patch clean

patch:
	@echo "If using HDF5-1.8, make sure you have patched repack"

plugin:
	cd ../src; $(MAKE) $(MAKEVARS) $@

lib:
	cd ../src; $(MAKE) $(MAKEVARS) $@

test_write_plugin.o: test_write.c
	$(CC) -c $< -o $@ -DH5Z_ZFP_USE_PLUGIN -DZFP_LIB_VERSION=0x$(ZFP_LIB_VERSION) $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_write_lib.o: test_write.c
	$(CC) -c $< -o $@ $(CFLAGS) -DZFP_HAS_CFP=$(ZFP_HAS_CFP) -DZFP_LIB_VERSION=0x$(ZFP_LIB_VERSION) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_write_plugin: test_write_plugin.o plugin
	$(CC) $< -o $@ $(CFLAGS) $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(HDF5_LIB) -L$(ZFP_LIB) -lhdf5 $(ZFP_LIBS) -lm $(LDFLAGS)

test_write_lib: test_write_lib.o lib
	$(CC) $< -o $@ $(CFLAGS) $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 $(ZFP_LIBS) -lm $(LDFLAGS)

test_read_plugin.o: test_read.c
	$(CC) -c $< -o $@ -DH5Z_ZFP_USE_PLUGIN $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_read_lib.o: test_read.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_read_plugin: test_read_plugin.o plugin
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(HDF5_LIB) -L$(ZFP_LIB) -lhdf5 $(ZFP_LIBS) $(LDFLAGS)

test_read_lib: test_read_lib.o lib
	$(CC) $< -o $@ $(CFLAGS) $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 $(ZFP_LIBS) $(LDFLAGS)

test_error.o: test_error.c
	$(CC) -c $< -o $@ $(CFLAGS) -DZFP_LIB_VERSION=0x$(ZFP_LIB_VERSION) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_error: test_error.o lib
	$(CC) $< -o $@ $(CFLAGS) $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 $(ZFP_LIBS) -lm $(LDFLAGS)

print_h5repack_farg: print_h5repack_farg.o
	$(CC) $< -o $@ $(LDFLAGS)

print_h5repack_farg.o: print_h5repack_farg.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(HDF5_INC)

ifneq ($(FC),) # Fortran Tests [

test_rw_fortran: test_rw_fortran.o lib
	$(FC) $(FCFLAGS) -o $@ $< $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5_fortran -lhdf5 $(ZFP_LIBS) $(LDFLAGS)

%.o:%.F90
	$(FC)  -o $@ -c $< $(FCFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)


# Note: The write-half of all the Fortran tests utilize the default and properties
# interface library to control the filter. The read-half uses the plugin.

test-default : test_rw_fortran
	 @echo " "; \
	 pad=$$(printf '%*s' "$(padlimit)"); \
	 pad=$${pad// /.}; \
         x="./test_rw_fortran"; \
	 printf "$$x"; \
	 printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
         $$x 2>&1 1>/dev/null; \
         if [[ $$status -ne 0 ]]; then \
	     printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	     touch check-failed; \
	     exit 0; \
          fi; \
	  printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \

# Decrease bit rate and confirm compression ratio increases
test-rate-f: plugin test_rw_fortran
	 @echo " "; \
	 pad=$$(printf '%*s' "$(padlimit)"); \
	 pad=$${pad// /.}; \
	 for r in 32 16 8 4; do\
            x="./test_rw_fortran zfpmode 1 rate $$r"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    expected_ratio=$$(expr 64 \/ $$r); \
	    $$x >/dev/null 2>/dev/null; \
            status=$$?; \
            if [[ $$status -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
            fi; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp_fortran.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DUMP*$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
	    fi; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed-plugin -p test_zfp_fortran.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DUMP*$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy-f: plugin test_rw_fortran
	 @echo " "; \
	 pad=$$(printf '%*s' "$(padlimit)"); \
	 pad=$${pad// /.}; \
	 for a in 0.1 0.01 0.001 0.0001; do\
            x="./test_rw_fortran zfpmode 3 acc $$a write"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x >/dev/null 2>/dev/null; \
            status=$$?; \
            if [[ $$status -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
            fi; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
	    fi; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp_fortran.h5 test_zfp_fortran.h5 compressed-plugin original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \

# Increase precision and confirm diff count for given tolerance drops
test-precision-f: plugin test_rw_fortran
	 @echo " "; \
	 pad=$$(printf '%*s' "$(padlimit)"); \
	 pad=$${pad// /.}; \
	 for p in 12 16 20 24; do\
            x="./test_rw_fortran zfpmode 2 prec $$p write"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x >/dev/null 2>/dev/null; \
            status=$$?; \
            if [[ $$status -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        touch check-failed; \
	        exit 0; \
            fi; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	        touch check-failed; \
		exit 0; \
	    fi; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed-plugin original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	        touch check-failed; \
		exit 0; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	 done; \

# Ensure reversible gives bit-for-bit identical results
test-reversible-f: plugin test_rw_fortran
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_rw_fortran zfpmode 5 write"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x >/dev/null 2>/dev/null; \
        status=$$?; \
        if [[ $$status -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
            exit 0; \
         fi; \
	diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	if [[ $$diffcnt -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed-plugin original | grep 'differences found' | cut -d' ' -f1); \
	if [[ $$diffcnt -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED *H5DIFF*$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n\n"; \

endif # ] ifneq ($(FC),)

# Decrease bit rate and confirm compression ratio increases
test-rate: plugin test_write_plugin
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	for r in 32 16 8 4; do \
	    expected_ratio=$$(expr 64 \/ $$r); \
	    x="./test_write_plugin zfpmode=1 rate=$$r"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
            fi; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Plugin rate tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy: plugin test_write_plugin
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	for a in 0.1 0.01 0.001 0.0001; do\
	    x="./test_write_plugin zfpmode=3 acc=$$a"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp.h5 test_zfp.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        echo "Plugin accuracy test failed for accuracy=$$a"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Plugin accuracy tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

# Increase precision and confirm diff count for given tolerance drops
test-precision: plugin test_write_plugin
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	ldiffcnt=0; \
	for p in 12 16 20 24; do\
	    x="./test_write_plugin zfpmode=2 prec=$$p"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
		continue; \
	    fi; \
	    ldiffcnt=$$diffcnt; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Plugin precision tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

# Ensure reversible mode works
test-reversible: plugin test_write_plugin
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_plugin zfpmode=5"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $$x 2>&1 1>/dev/null; \
        if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	if [[ $$diffcnt -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

#
# Uses h5repack to test ZFP filter on float and int datasets in
# 1,2,3 and 4 dimensions. Note: need to specify raw cd_values on
# command-line to h5repack. We can get these from an invokation
# of test_write which prints them in the header output. The values
# here are for accuracy mode and tolerance of 0.001.
# 
# A bug-fix patch to h5repack_parse.c is required for this test on
# older HDF5 versions (like <= 1.8.10).
#
test-h5repack: plugin mesh.h5 patch
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="h5repack -f UD=32013,0,4,3,0,3539053052,1062232653"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/$$x \
	     -l X,Y,Z,Indexes:CHUNK=217 \
	     -l Indexes2:CHUNK=1517 \
	     -l Pressure,Pressure2,Pressure3:CHUNK=10x20x5 \
	     -l Pressure_2D:CHUNK=10x20 \
	     -l Stress,Velocity,Stress2,Velocity2,Stress3,Velocity3,VelocityZ,VelocityZ2,VelocityZ3:CHUNK=11x21x1x1 \
	     -l VelocityX_2D:CHUNK=21x31 \
	     -l XY:CHUNK=651x1 \
	     -l XYZ:CHUNK=217x1 \
	     -l XYZ2:CHUNK=1617x1 \
	     -l XYZ3:CHUNK=33x1 \
	     mesh.h5 mesh_repack.h5 2>&1 1>/dev/null; \
        if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	orig_size=$$(ls -l mesh.h5 | tr -s ' ' | cut -d' ' -f5); \
	new_size=$$(ls -l mesh_repack.h5 | tr -s ' ' | cut -d' ' -f5); \
        ratio=$$(perl -e "printf int($$orig_size*100/$$new_size)"); \
	if [[ $$ratio -lt 200 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED (size ratio) $(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

# Diff ZFP compressed data from little endian and big endian machines
# There is a bug in h5diff that causes it to return 0 when it can't find plugin.
# We protect against that by additional check of output error text
# We should test return code but h5diff's return code has been unreliable across
# versions and so am not relying on it here.
test-endian: plugin test_zfp_le.h5 test_zfp_be.h5
	@echo " "; \
        padlimit=80; \
        padlength=80; \
	pad=$$(printf '%*s' "$${padlimit}"); \
	pad=$${pad// /.}; \
        x="h5diff -v -d 0.00001 test_zfp_le.h5 test_zfp_be.h5 compressed compressed"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($${padlength} - $${#x} )) "$$pad"; \
	outerr=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/$$x 2>&1); \
	if [[ -z "$$(echo $$outerr | grep '0 differences found')" ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

# Test the filter as a library rather than as a plugin
test-lib-rate: test_write_lib test_read_lib
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	for t in 32:1e-07 16:0.003 8:0.4; do\
	    r=$$(echo $$t | cut -d':' -f1); \
	    d=$$(echo $$t | cut -d':' -f2); \
	    x="./test_write_lib rate=$$r zfpmode=1"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    ./test_read_lib max_absdiff=$$d max_reldiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED (readback)$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Library rate tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-lib-accuracy: test_write_lib test_read_lib
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	for v in 0.1:0.025 0.01:0.004 0.001:0.0006 0.0001:4e-5; do\
	    a=$$(echo $$v | cut -d':' -f1); \
	    d=$$(echo $$v | cut -d':' -f2); \
	    x="./test_write_lib acc=$$a zfpmode=3"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    ./test_read_lib ret=1 max_absdiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED (readback)$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Library accuracy tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-lib-precision: test_write_lib test_read_lib
	@failed=0; \
	echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	for v in 12:0.02 16:0.0005 20:5e-5 24:1e-6; do\
	    p=$$(echo $$v | cut -d':' -f1); \
	    d=$$(echo $$v | cut -d':' -f2); \
	    x="./test_write_lib prec=$$p zfpmode=2"; \
            printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x 2>&1 1>/dev/null; \
            if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    ./test_read_lib ret=2 max_reldiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        printf " [$(ERROR_COLOR)FAILED (readback)$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Library precision tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-lib-reversible: test_write_lib test_read_lib
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_lib zfpmode=5"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	./test_read_lib ret=1 max_absdiff=0 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED (readback)$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-int: test_write_lib test_read_lib
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_lib zfpmode=3 doint=1"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	status=$$?; \
	if [[ $$status -eq 1 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	elif [[ $$status -eq 2 ]]; then \
	    printf " [$(SKIP_COLOR)SKIPPED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	./test_read_lib ret=1 max_absdiff=2 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED (readback)$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-highd: test_write_lib
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_lib highd=1"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-sixd: test_write_lib
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_lib sixd=1"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	status=$$?; \
	if [[ $$status -eq 2 ]]; then \
	    printf " [$(SKIP_COLOR)SKIPPED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	elif [[ $$status -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-error: test_error
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_error"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-zfparr: test_write_lib
	@echo " "; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	x="./test_write_lib zfparr=1 rate=10"; \
	printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	$$x 2>&1 1>/dev/null; \
	status=$$?; \
	if [[ $$status -eq 2 ]]; then \
	    printf " [$(SKIP_COLOR)SKIPPED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	elif [[ $$status -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d 0.01 test_zfp.h5 test_zfp.h5 zfparr_original zfparr_direct 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED (h5diff) $(NO_COLOR)]\n"; \
	    touch check-failed; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

test-version-compatibility: test_read_lib
	@echo " "; \
	failed=0; \
	pad=$$(printf '%*s' "$(padlimit)"); \
	pad=$${pad// /.}; \
	files="test_zfp_030040.h5:0 test_zfp_030235.h5:0 test_zfp_110050.h5:0 test_zfp_110xxx.h5:1"; \
	if [[ -x ./test_rw_fortran ]]; then \
	    ./test_rw_fortran write dim 1024 zfpmode 1 rate 16 2>&1 1>/dev/null; \
	    if [[ $$? -eq 0 ]]; then \
	        files="$$files test_zfp_fortran.h5:0"; \
	    fi; \
	fi; \
	for f in $$files; do \
	    fname=$$(echo $$f | cut -d':' -f1); \
	    xstat=$$(echo $$f | cut -d':' -f2); \
	    x="./test_read_lib ifile=$$fname max_reldiff=0.025"; \
	    printf "$$x"; \
	    printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
	    $$x ret=2 2>/dev/null 1>/dev/null; \
	    if [[ $$? -ne $$xstat ]]; then \
	        printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	        failed=1; \
	        touch check-failed; \
	        continue; \
	    fi; \
	    printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"; \
	done; \
	x="Version compatibility tests"; \
        printf "$$x"; \
	printf ' %*.*s' 0 $$(($(padlength) - $${#x} )) "$$pad"; \
        if [[ $$failed -ne 0 ]]; then \
	    printf " [$(ERROR_COLOR)FAILED$(NO_COLOR)]\n"; \
	    exit 0; \
	fi; \
	printf " [$(OK_COLOR)PASSED$(NO_COLOR)] \n"

check-start:
	@rm -f check-failed

check-end:
	@if [[ -e check-failed ]]; then \
	    exit 1; \
	fi

SPECIAL_CHECK = test-version-compatibility test-int test-highd test-sixd test-zfparr

FORTRAN_CHECK = test-default test-rate-f test-accuracy-f test-precision-f
ifneq ($(ZFP_HAS_REVERSIBLE),)
    FORTRAN_CHECK += test-reversible-f
endif

LIB_CHECK = test-lib-rate test-lib-accuracy test-lib-precision test-error
ifneq ($(ZFP_HAS_REVERSIBLE),)
    LIB_CHECK += test-lib-reversible
endif

PLUGIN_CHECK = test-rate test-accuracy test-precision test-h5repack
ifneq ($(ZFP_HAS_REVERSIBLE),)
    PLUGIN_CHECK += test-reversible
endif
PLUGIN_CHECK += test-endian

CHECK = $(LIB_CHECK) $(PLUGIN_CHECK)
ifneq ($(FC),)
    CHECK += $(FORTRAN_CHECK)
endif
CHECK += $(SPECIAL_CHECK)

check: check-start $(CHECK) check-end

clean:
	rm -f test_write_plugin.o test_write_lib.o test_read_plugin.o test_read_lib.o test_rw_fortran.o test_error.o
	rm -f test_write_plugin test_write_lib test_read_plugin test_read_lib test_rw_fortran test_error
	rm -f test_zfp.h5 test_zfp_fortran.h5 mesh_repack.h5 test_zfp_errors.h5
	rm -f print_h5repack_farg.o print_h5repack_farg check-failed
	rm -f *.gcno *.gcda *.gcov
