

Program which generates some Poisson noise and then measures time to read it back.

Check whether you have linked ipp:

$  nm `python -c 'import hdf5plugin;print(hdf5plugin.PLUGINS_PATH)'`/libh5bshuf.so | grep ippsDecodeLZ4_8u

Run it from hdf5plugin source dir (to locate cpuinfo.py):

$ python bench/eiger4m.py bench_cases /dev/shm 100 | tee ./bench/`uname -n`_tag_whatever_you_like
 
