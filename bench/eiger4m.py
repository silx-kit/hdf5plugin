

import timeit
import sys
import os
import pprint
import numpy as np
import hdf5plugin
import h5py
# cpuinfo is in hdf5plugin root folder (one up from bench)
sys.path.append( os.path.join( os.path.dirname( __file__ ), ".." ) )
import cpuinfo

G = 1e-9 # decimal

def make_signal( shape, signal ):
    return np.random.poisson( signal, shape ) 

def module(a,i,j):
    fs = 513
    ss = 512
    fo = [0,2,14,16]
    so = [0,38,2*38,38*3]
    f = fs*j + fo[j]
    s = ss*i + so[i]
    return a[s:s+ss,f:f+fs]

def make_mask( shape ):
    msk = np.zeros( shape, np.uint8 )
    for i in range(4):
        for j in range(4):
            module(msk, i, j)[:] = 1
    nbad = int(msk.size*0.001)
    dead = np.random.randint(0, high=msk.size, size=nbad )
    msk.flat[dead]=0
    return msk

def bench_write( h5name, dsname, frms ):
    t0 = timeit.default_timer()
    with h5py.File(h5name, "w") as h:
        ds = h.create_dataset( dsname,
                               data = frms,
                               chunks = (1,frms.shape[1],frms.shape[2]),
                               **hdf5plugin.Bitshuffle(nelems=0, lz4=True),
                              )
        h.flush()
        h.close()
    t1 = timeit.default_timer()
    fs = os.stat( h5name ).st_size*G
    ratio = frms.nbytes*G / fs
    print("wrote",h5name, dsname, "%.3f GB"%(frms.nbytes*G),
          frms.shape, "%.3f s %.3f GB %.1f:1"%( t1-t0, fs, ratio ) )
  
def bench_read( h5name, dsname, rpt=10 ):
    dt = []
    for i in range( rpt ):
        t0 = timeit.default_timer()
        with h5py.File(h5name, "r") as h:
            frms = h[dsname][:]
        t1 = timeit.default_timer()
        dt.append( t1 - t0 )
    best = np.min( dt )
    gb = frms.nbytes*G
    print("read", h5name, dsname, frms.dtype,
          "%.3f GB %.3f s %.3f s %.3f GB/s"%(
              gb, best, np.max(dt), gb / best ) )
    sys.stdout.flush()
    return frms

def bench_cases( path, nframes = 100 ):
    print("=== cpuinfo ===")
    pprint.pprint( cpuinfo.get_cpu_info() )
    print("=== plugin config ===")
    print(hdf5plugin.config)
    NX = 2068
    NY = 2162
    shape = (NY, NX)
    msk = make_mask( shape )
    for signal in (0.001, 1, 1000):
        for dtype in (np.uint16, np.uint32):
            frm0 = make_signal( shape, signal ).astype( dtype )
            frms = np.empty( (int(nframes), NY, NX ), dtype )
            print("=== Generating test data", signal, frms.dtype, shape,"===")
            for i, frm in enumerate(frms):
                frm[:] = np.roll( frm0, (i*7)%NY, axis=0 )*msk
            h5name = os.path.join(path, "bench.h5")
            dsname = 'testdata'
            bench_write( h5name, dsname, frms )
            nth = 1
            while nth <= os.cpu_count():
                os.environ['OMP_NUM_THREADS'] = str(nth)
                print('OMP_NUM_THREADS', os.environ['OMP_NUM_THREADS'], end=' ')
                sys.stdout.flush()
                ret = os.system("%s %s bench_read %s %s"%(
                    sys.executable, __file__, h5name, dsname ) )
                if ret != 0:
                    raise Exception("Error reading")
                nth *= 2
            print("TESTREAD",end = ' ')
            tst = bench_read( h5name, dsname )
            assert (tst == frms).all()
             
        
if __name__=="__main__":
    np.random.seed(10007*10009)
    func = globals()[sys.argv[1]]
    args = tuple( sys.argv[2:] )
    func( *args )

#  hdf5plugin$ python bench/eiger4m.py bench_cases /dev/shm 100 | tee ./bench/`uname -n` 
