==============
 Installation
==============

Pre-built packages
------------------

Pre-built binaries of `hdf5plugin` are available from:

- `pypi <https://pypi.org/project/hdf5plugin>`_, to install run:
  ``pip install hdf5plugin [--user]``
- `conda-forge <https://anaconda.org/conda-forge/hdf5plugin>`_, to install run:
  ``conda install -c conda-forge hdf5plugin``

To maximize compatibility, those binaries are built without optimization options (such as `AVX2`_ and `OpenMP`_).
`Installation from source`_ can achieve better performances than pre-built binaries.

Installation from source
------------------------

The build process enables compilation optimizations that are supported by the host machine.

To install from source and recompile the HDF5 plugins, run::

    pip install hdf5plugin --no-binary hdf5plugin [--user]

To override the defaults that are probed from the machine, it is possible to specify build options.
This is achieved by either setting environment variables or passing options to ``python setup.py build``, for example:

- ``HDF5PLUGIN_OPENMP=False pip install hdf5plugin --no-binary hdf5plugin``
- From the source directory: ``python setup.py build --openmp=False``

Available options
.................

.. list-table::
   :widths: 1 1 4
   :header-rows: 1

   * - Environment variable
     - ``python setup.py build`` option
     - Description
   * - ``HDF5PLUGIN_HDF5_DIR``
     - ``--hdf5``
     - Custom path to HDF5 (as in h5py).
   * - ``HDF5PLUGIN_OPENMP``
     - ``--openmp``
     - Whether or not to compile with `OpenMP`_.
       Default: True if probed (always False on macOS).
   * - ``HDF5PLUGIN_NATIVE``
     - ``--native``
     - True to compile specifically for the host, False for generic support (For unix compilers only).
       Default: True on supported architectures, False otherwise
   * - ``HDF5PLUGIN_SSE2``
     - ``--sse2``
     - Whether or not to compile with `SSE2`_ support.
       Default: True on ppc64le and when probed on x86, False otherwise
   * - ``HDF5PLUGIN_AVX2``
     - ``--avx2``
     - Whether or not to compile with `AVX2`_ support. avx2=True requires sse2=True.
       Default: True on x86 when probed, False otherwise
   * - ``HDF5PLUGIN_CPP11``
     - ``--cpp11``
     - Whether or not to compile C++11 code if available.
       Default: True if probed.

Note: Boolean options are passed as ``True`` or ``False``.


.. _AVX2: https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2
.. _SSE2: https://en.wikipedia.org/wiki/SSE2
.. _OpenMP: https://www.openmp.org/
