.. _hdf5_chunking:

==============
HDF5_ Chunking
==============

HDF5_'s dataset `chunking`_ feature is a way to optimize data layout on disk to support partial dataset reads by downstream consumers.
This is all the more important when compression filters are applied to datasets as it frees a consumer from suffering the UNcompression of an entire dataset only to read a portion.

-------------
ZFP Chunklets
-------------

When using HDF5_ `chunking`_ with ZFP_ compression, it is important to account for the fact that ZFP_ does its work in tiny 4\ :sup:`d` chunklets of its own where `d` is the dataset dimension (*rank* in HDF5_ parlance).
This means that whenever possible, the `chunking`_ dimensions you select in HDF5_ should be multiples of 4.
When a chunk_ dimension is not a multiple of 4, ZFP_ will wind up with partial chunklets, which will be padded with useless data, reducing the results' overall time and space efficiency.

The degree to which this may degrade performance depends on the percentage of a chunk_ that is padded.
Suppose we have a 2D chunk of dimensions 27 x 101.
ZFP_ will have to treat it as 28 x 104 by padding out each dimension to the next closest multiple of 4.
The fraction of space that will wind up being wasted due to ZFP_ chunklet padding will be (28x104-27x101) / (28x104), which is about 6.4%.
On the other hand, consider a 3D chunk that is 1024 x 1024 x 2.
ZFP_ will have to treat it as a 1024 x 1024 x 4 resulting in 50% waste.

The latter example is potentially very relevant when applying ZFP_ to compress data along the *time* dimension in a large, 3D, simulation.
Ordinarily, a simulation advances one time step at a time and so needs to store in memory only the *current* timestep.
However, in order to give ZFP_ enough *width* in the time dimension to satisfy the minimum chunklet dimension size of 4, the simulation
needs to keep in memory 4 timesteps.
This is demonstrated in the example below.

--------------------
Partial I/O Requests
--------------------

In any given H5Dwrite_ call, the caller has the option of writing (or reading) only a portion of the data in the dataset.
This is a *partial I/O* request.
This is handled by the ``mem_space_id`` and ``file_space_id`` arguments in an H5Dwrite_ call.

An HDF5_ producer or consumer can issue partial I/O requests on *any* HDF5 dataset regardless of whether the dataset is compressed or not or whether the dataset has ``H5D_CONTIGUOUS`` layout.
When combining partial I/O with compression, chunk size and shape in relation to partial I/O request size and shape will have an impact on performance.

This is particularly important in *writer* scenarios if an I/O request winds up overlapping chunks only partially.
Suppose the partially overlapped chunks exist in the file (from a previous write, for example). In that case, the HDF5_ library may wind up having to engage in *read-modify-write* operations for those chunks.

If the partially overlapped chunks do not exist in the file, the HDF5_ library will wind up *fill-value* padding the chunks before they are written.
HDF5_'s default fill value is zero (as defined by the associated datatype).
Data producers can choose the desired fill value (see `H5Pset_fill_value <https://docs.hdfgroup.org/hdf5/develop/group___d_c_p_l.html#ga4335bb45b35386daa837b4ff1b9cd4a4>`__) for a dataset, but this fill value can impact the space-performance of the compression filter.
On the other hand, if the partial chunks in one I/O request wind up getting fully filled in another, any fill value impacts on compressor performance are resolved.

Finally, HDF5_ manages a `chunk cache <https://docs.hdfgroup.org/hdf5/develop/group___f_a_p_l.html#ga034a5fc54d9b05296555544d8dd9fe89>`__ and `data sieving buffer <https://docs.hdfgroup.org/hdf5/develop/group___f_a_p_l.html#ga24fd737955839194bf5605d5f47928ee>`__ to help alleviate some of the I/O performance issues that can be encountered in these situations.

-----------------------------
More Than 3 (or 4) Dimensions
-----------------------------

Versions of ZFP_ 0.5.3 and older support compression in only 1,2 or 3
dimensions. Versions of ZFP_ 0.5.4 and newer also support 4 dimensions.

What if you have a dataset with more dimensions than ZFP_ can compress?
You can still use the H5Z-ZFP_ filter. But, in order to do so, you
are *required* to chunk_ the dataset [1]_ . Furthermore, you must select a 
chunk_ size such that no more than 3 (or 4 for ZFP_ 0.5.4 and newer)
dimensions are non-unitary (e.g. of size one). 

For example, what if you are using ZFP_ 0.5.3 and have a 4D HDF5 dataset
you want to compress? To do this, you will need to chunk_ the dataset and
when you define the chunk_ size and shape, you will need to select which
of the 4 dimensions of the chunk you do *not* intend to have ZFP_ perform
compression along by setting the size of the chunk_ in that dimension to
unity (1). When you do this, as HDF5 processes writes and reads, it will
organize the data so that all the H5Z-ZFP_ filter *sees* are chunks
which have *extent* only in the non-unity dimensions of the chunk_.

In the example below, we have a 4D array of shape ``int dims[] = {256,128,32,16};``
that we have intentionally constructed to be *smooth* in only 2 of its 4 dimensions
(e.g. correlation is high in those dimensions). Because of that, we expect ZFP_
compression to do well along those dimensions, and we do not want ZFP_ to compress
along the other 2 dimensions. The *uncorrelated* dimensions here are dimensions
with indices ``1`` (``128`` in ``dims[]``) and ``3`` (``16`` in ``dims[]``). 
Thus, our chunk_ size and shape are chosen to set the size for those dimension
indices to ``1``, ``hsize_t hchunk[] = {256,1,32,1};``

.. literalinclude:: ../test/test_write.c
   :language: c
   :linenos:
   :start-after: Test high dimensional (>3D) array
   :end-before: End of high dimensional test

What analysis process should you use to select the chunk_ shape? Depending
on what you expect in the way of access patterns in downstream consumers,
this can be a challenging question to answer. There are potentially two
competing interests. One is optimizing the chunk_ size and shape for access
patterns anticipated by downstream consumers. The other is optimizing the chunk_
size and shape for compression. These two interests may not be compatible
and you may have to compromise between them. We illustrate the issues and
trade-offs using an example.

---------------------------------------------------
Compression *Along* the *State Iteration* Dimension 
---------------------------------------------------

By *state iteration* dimension, we refer to the data producer's main iteration
loop(s). For example, the main iteration dimension for many
PDE-based simulations is *time*. But, for some *outer loop* methods, the
main iteration dimension(s) might be some kind of parameter study including
multiple parameters.

The challenge here is to manage the data to meet ZFP_'s
chunklet size and shape *minimum* requirements. In any H5Dwrite_ at least 4
*samples* along a ZFP_ compression dimension are needed, or there will
be wasted space due to padding. This means that data must be *buffered* 
along those dimensions *before* H5Dwrite_'s can be issued.

For example, suppose you have a tensor-valued field (e.g. a 3x3 matrix
at every *point*) over a 4D (3 spatial dimensions and 1 time dimension),
regularly sampled domain? Conceptually, this is a 6 dimensional dataset
in HDF5_ with one of the dimensions (the *time* dimension) *extendable*.
So, you are free to define this as a 6 dimensional dataset in HDF5_. But, you
will also have to chunk_ the dataset. You can select any chunk_ shape
you want, except that no more than 3 (or 4 for ZFP_ versions 0.5.4 and
newer) dimensions of the chunk_ can be non-unity.

In the code snippet below, we demonstrate this case. A key issue to deal
with is that because we will use ZFP_ to compress along the time dimension,
this forces us to keep in memory a sufficient number of timesteps to match
ZFP_'s chunklet size of 4.

The code below iterates over 9 timesteps. Each of the first two groups of 4
timesteps are buffered in memory in ``tbuf``. Once 4 timesteps have been buffered, we
can issue an H5Dwrite_ call doing
`hyperslab <https://docs.hdfgroup.org/hdf5/develop/_h5_d__u_g.html#subsubsec_dataset_transfer_partial>`__
can issue an H5Dwrite_
call doing `hyperslab <https://docs.hdfgroup.org/hdf5/develop/_h5_d__u_g.html#subsubsec_dataset_transfer_partial>`__
partial I/O on the 6D, `extendable <https://docs.hdfgroup.org/hdf5/develop/_l_b_ext_dset.html>`__
dataset. But, notice that the chunk_ dimensions (line 10) are such that only 4 of the
6 dimensions are non-unity. This means ZFP_ will only ever see something to
compress that is essentially 4D.

On the last iteration, we have only one *new* timestep. So, when we write this
to ZFP_ 75% of that write will be *wasted* due to ZFP_ chunklet padding. However,
if the application were to *restart* from this time and continue forward, this
*waste* would ultimately get overwritten with new timesteps.

.. literalinclude:: ../test/test_write.c
   :language: c
   :linenos:
   :start-after: 6D Example
   :end-before: End of 6D Example

.. _chunking: https://portal.hdfgroup.org/display/HDF5/Chunking+in+HDF5
.. _chunk: https://portal.hdfgroup.org/display/HDF5/Chunking+in+HDF5
.. _H5Dwrite: https://docs.hdfgroup.org/hdf5/v1_14/group___h5_d.html#title37
.. [1] The HDF5_ library currently requires dataset chunking anyways for
   any dataset that has any kind of filter applied.

