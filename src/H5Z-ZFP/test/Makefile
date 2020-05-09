include ../config.make

.PHONY: lib plugin check patch clean

patch:
	@echo "Make sure you have patched HDF5's repack tool"

plugin:
	cd ../src; $(MAKE) $(MAKEVARS) $@

lib:
	cd ../src; $(MAKE) $(MAKEVARS) $@

test_write_plugin.o: test_write.c
	$(CC) -c $< -o $@ -DH5Z_ZFP_USE_PLUGIN $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_write_lib.o: test_write.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_write_plugin: test_write_plugin.o plugin
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(HDF5_LIB) -L$(ZFP_LIB) -lhdf5 -lzfp -lm $(LDFLAGS)

test_write_lib: test_write_lib.o lib
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 -lzfp -lm $(LDFLAGS)

test_read_plugin.o: test_read.c
	$(CC) -c $< -o $@ -DH5Z_ZFP_USE_PLUGIN $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_read_lib.o: test_read.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_read_plugin: test_read_plugin.o plugin
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L$(HDF5_LIB) -L$(ZFP_LIB) -lhdf5 -lzfp $(LDFLAGS)

test_read_lib: test_read_lib.o lib
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 -lzfp $(LDFLAGS)

test_error.o: test_error.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

test_error: test_error.o lib
	$(CC) $< -o $@ $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5 -lzfp -lm $(LDFLAGS)

print_h5repack_farg: print_h5repack_farg.o
	$(CC) $< -o $@ $(LDFLAGS)

print_h5repack_farg.o: print_h5repack_farg.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(HDF5_INC)

ifneq ($(FC),) # Fortran Tests [

test_rw_fortran: test_rw_fortran.o lib
	$(FC) $(FCFLAGS) -o $@ $< $(PREPATH)$(HDF5_LIB) $(PREPATH)$(ZFP_LIB) -L../src -L$(HDF5_LIB) -L$(ZFP_LIB) -lh5zzfp -lhdf5_fortran -lhdf5 -lzfp $(LDFLAGS)
	./test_rw_fortran

%.o:%.F90
	$(FC)  -o $@ -c $< $(FCFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)


# Note: The write-half of all the Fortran tests utilize the default and properties
# interface library to control the filter. The read-half uses the plugin.

# Decrease bit rate and confirm compression ratio increases
test-rate-f: plugin test_rw_fortran 
	@for r in 32 16 8 4; do\
	    expected_ratio=$$(expr 64 \/ $$r); \
	    ./test_rw_fortran zfpmode 1 rate $$r 2>&1 1>/dev/null; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp_fortran.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        echo "Fortran rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Fortran Rate tests passed"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy-f: plugin test_rw_fortran
	@for a in 0.1 0.01 0.001 0.0001; do\
	    ./test_rw_fortran zfpmode 3 acc $$a write_only 2>&1 1>/dev/null; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Fortran accuracy test failed for accuracy=$$a"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Fortran Accuracy tests passed"

# Increase precision and confirm diff count for given tolerance drops
test-precision-f: plugin test_rw_fortran
	@ldiffcnt=0; \
	for p in 12 16 20 24; do\
	    ./test_rw_fortran zfpmode 2 prec $$p write_only 2>&1 1>/dev/null; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        echo "Fortran precision test failed for precision=$$p"; \
		exit 1; \
	    fi; \
	    ldiffcnt=$$diffcnt; \
	done; \
	echo "Fortran Precision tests passed"

# Ensure reversible gives bit-for-bit identical results
test-reversible-f: plugin test_rw_fortran
	@./test_rw_fortran zfpmode 5 write_only 2>&1 1>/dev/null; \
	diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp_fortran.h5 test_zfp_fortran.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	if [[ $$diffcnt -ne 0 ]]; then \
	    echo "Fortran Reversible test failed"; \
	    exit 1; \
	fi; \
	echo "Fortran Reversible test passed"

endif # ] ifneq ($(FC),)

# Decrease bit rate and confirm compression ratio increases
test-rate: plugin test_write_plugin
	@for r in 32 16 8 4; do\
	    expected_ratio=$$(expr 64 \/ $$r); \
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write_plugin zfpmode=1 rate=$$r 2>&1 1>/dev/null; \
	    actual_ratio=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5dump -H -d compressed -p test_zfp.h5 | grep COMPRESSION | cut -d':' -f1 | cut -d'(' -f2 | cut -d'.' -f1); \
	    if [[ $$expected_ratio -ne $$actual_ratio ]]; then \
	        echo "Plugin rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Plugin rate tests passed"

# Increase accuracy and test absolute error tolerance is within accuracy
test-accuracy: plugin test_write_plugin
	@for a in 0.1 0.01 0.001 0.0001; do\
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write_plugin zfpmode=3 acc=$$a 2>&1 1>/dev/null; \
	    env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d $$a test_zfp.h5 test_zfp.h5 compressed original 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Plugin accuracy test failed for accuracy=$$a"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Plugin accuracy tests passed"

# Increase precision and confirm diff count for given tolerance drops
test-precision: plugin test_write_plugin
	@ldiffcnt=0; \
	for p in 12 16 20 24; do\
	    env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write_plugin zfpmode=2 prec=$$p 2>&1 1>/dev/null; \
	    diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	    if [[ $$ldiffcnt -ne 0 ]] && [[ $$diffcnt -gt $$ldiffcnt ]]; then \
	        echo "Plugin precision test failed for precision=$$p"; \
		exit 1; \
	    fi; \
	    ldiffcnt=$$diffcnt; \
	done; \
	echo "Plugin precision tests passed"

# Ensure reversible mode works
test-reversible: plugin test_write_plugin
	@env HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) ./test_write_plugin zfpmode=5 2>&1 1>/dev/null; \
	diffcnt=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -p 0.00001 test_zfp.h5 test_zfp.h5 compressed original | grep 'differences found' | cut -d' ' -f1); \
	if [[ $$diffcnt -ne 0 ]]; then \
	    echo "Plugin reversible test failed"; \
	    exit 1; \
	fi; \
	echo "Plugin reversible tests passed"

#
# Uses h5repack to test ZFP filter on float and int datasets in
# 1,2,3 and 4 dimensions. Note: need to specify raw cd_values on
# command-line to h5repack. We can get these from an invokation
# of test_write which prints them in the header output. The values
# here are for accuracy mode and tolerance of 0.001.
# 
# A bug-fix patch to h5repack_parse.c is required for this test.
#
test-h5repack: plugin mesh.h5 patch
	@env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5repack -f UD=32013,6,3,0,3539053052,1062232653,0,0 \
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
	    echo "Repack command failed."; \
	    echo "Did you patch h5repack? See README."; \
	    exit 1; \
        fi; \
	orig_size=$$(ls -l mesh.h5 | tr -s ' ' | cut -d' ' -f5); \
	new_size=$$(ls -l mesh_repack.h5 | tr -s ' ' | cut -d' ' -f5); \
        ratio=$$(echo "$$orig_size\n100\n*\n$$new_size\n/p\n" | dc -); \
	if [[ $$ratio -lt 2 ]]; then \
	    echo "ZFP Repack Test failed"; \
	    exit 1; \
	fi; \
	echo "Repack test passed"

# Diff ZFP compressed data from little endian and big endian machines
# There is a bug in h5diff that causes it to return 0 when it can't find plugin.
# We protect against that by additional check of output error text
# We should test return code but h5diff's return code has been unreliable across
# versions and so am not relying on it here.
test-endian: plugin test_zfp_le.h5 test_zfp_be.h5
	@outerr=$$(env LD_LIBRARY_PATH=$(HDF5_LIB):$(LD_LIBRARY_PATH) HDF5_PLUGIN_PATH=$(H5Z_ZFP_PLUGIN) $(HDF5_BIN)/h5diff -v -d 0.00001 test_zfp_le.h5 test_zfp_be.h5 compressed compressed 2>&1); \
	if [[ -z "$$(echo $$outerr | grep '0 differences found')" ]]; then \
	    echo "Endian test failed"; \
	    exit 1; \
	fi; \
	echo "Endian test passed"

# Test the filter as a library rather than as a plugin
test-lib-rate: test_write_lib test_read_lib
	@for t in 32:1e-07 16:0.003 8:0.4; do\
	    r=$$(echo $$t | cut -d':' -f1); \
	    d=$$(echo $$t | cut -d':' -f2); \
	    ./test_write_lib rate=$$r zfpmode=1 2>&1 1>/dev/null; \
	    ./test_read_lib max_absdiff=$$d max_reldiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Library rate test failed for rate=$$r"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Library rate tests passed"

test-lib-accuracy: test_write_lib test_read_lib
	@for v in 0.1:0.025 0.01:0.004 0.001:0.0006 0.0001:4e-5; do\
	    a=$$(echo $$v | cut -d':' -f1); \
	    d=$$(echo $$v | cut -d':' -f2); \
	    ./test_write_lib acc=$$a zfpmode=3 2>&1 1>/dev/null; \
	    ./test_read_lib ret=1 max_absdiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Library accuracy test failed for acc=$$a"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Library accuracy tests passed"

test-lib-precision: test_write_lib test_read_lib
	@for v in 12:0.02 16:0.0005 20:5e-5 24:1e-6; do\
	    p=$$(echo $$v | cut -d':' -f1); \
	    d=$$(echo $$v | cut -d':' -f2); \
	    ./test_write_lib prec=$$p zfpmode=2 2>&1 1>/dev/null; \
	    ./test_read_lib ret=2 max_reldiff=$$d 2>&1 1>/dev/null; \
	    if [[ $$? -ne 0 ]]; then \
	        echo "Library precision test failed for prec=$$p"; \
	        exit 1; \
	    fi; \
	done; \
	echo "Library precision tests passed"

test-lib-reversible: test_write_lib test_read_lib
	@./test_write_lib zfpmode=5 2>&1 1>/dev/null; \
	./test_read_lib ret=1 max_absdiff=0 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "Library reversible test failed"; \
	    exit 1; \
	fi; \
	echo "Library reversible tests passed"

test-int: test_write_lib test_read_lib
	@./test_write_lib doint=1 2>&1 1>/dev/null; \
	./test_read_lib ret=1 max_absdiff=2 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "Integer test failed"; \
	    exit 1; \
	fi; \
	echo "Integer test passed"

test-highd: test_write_lib
	@./test_write_lib highd=1 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "4D test failed"; \
	    exit 1; \
	fi; \
	echo "4D test passed"

test-sixd: test_write_lib
	@./test_write_lib sixd=1 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "6D, extendible test failed"; \
	    exit 1; \
	fi; \
	echo "6D, extendible test passed"

test-error: test_error
	@./test_error 2>&1 1>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
            echo "Error check test failed"; \
            exit 1; \
        fi; \
        echo "Error check test passed"

test-special: test-int test-highd test-sixd

FORTRAN_CHECK = test-rate-f test-accuracy-f test-precision-f
ifneq ($(ZFP_HAS_REVERSIBLE),)
    FORTRAN_CHECK += test-reversible-f
endif

LIB_CHECK = test-lib-rate test-lib-accuracy test-lib-precision test-special test-error
ifneq ($(ZFP_HAS_REVERSIBLE),)
    LIB_CHECK += test-lib-reversible
endif

PLUGIN_CHECK = test-rate test-accuracy test-precision
ifneq ($(ZFP_HAS_REVERSIBLE),)
    PLUGIN_CHECK += test-reversible
endif
PLUGIN_CHECK += test-endian

CHECK = $(LIB_CHECK) $(PLUGIN_CHECK)
ifneq ($(FC),)
    CHECK += $(FORTRAN_CHECK)
endif

check: $(CHECK)

clean:
	rm -f test_write_plugin.o test_write_lib.o test_read_plugin.o test_read_lib.o test_rw_fortran.o test_error.o
	rm -f test_write_plugin test_write_lib test_read_plugin test_read_lib test_rw_fortran test_error
	rm -f test_zfp.h5 test_zfp_fortran.h5 mesh_repack.h5
	rm -f print_h5repack_farg.o print_h5repack_farg
	rm -f *.gcno *.gcda *.gcov
