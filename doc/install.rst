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

To override the defaults that are probed from the machine,
it is possible to specify build options by setting environment variables, for example:

- ``HDF5PLUGIN_OPENMP=False pip install hdf5plugin --no-binary hdf5plugin``
- From the source directory: ``HDF5PLUGIN_OPENMP=False pip install .``

Available options
.................

.. list-table::
   :widths: 1 4
   :header-rows: 1

   * - Environment variable
     - Description
   * - ``HDF5PLUGIN_HDF5_DIR``
     - Custom path to HDF5 (as in h5py).
   * - ``HDF5PLUGIN_OPENMP``
     - Whether or not to compile with `OpenMP`_.
       Default: True if probed (always False on macOS).
   * - ``HDF5PLUGIN_NATIVE``
     - True to compile specifically for the host, False for generic support (For unix compilers only).
       Default: True on supported architectures, False otherwise
   * - ``HDF5PLUGIN_SSE2``
     - Whether or not to compile with `SSE2`_ support.
       Default: True on ppc64le and when probed on x86, False otherwise
   * - ``HDF5PLUGIN_AVX2``
     - Whether or not to compile with `AVX2`_ support.
       It requires enabling `SSE2`_ support.
       Default: True on x86 when probed, False otherwise
   * - ``HDF5PLUGIN_AVX512``
     - Whether or not to compile with `AVX512`_ Foundation (F) and Byte and Word (BW) instruction sets support.
       It requires enabling `SSE2`_ and `AVX2`_ support.
       Default: True on x86 when probed, False otherwise
   * - ``HDF5PLUGIN_BMI2``
     - Whether or not to compile Zstandard with `BMI2`_ support if available.
       Default: True on Linux and MacOS, False otherwise
   * - ``HDF5PLUGIN_CPP11``
     - Whether or not to compile C++11 code if available.
       Default: True if probed.
   * - ``HDF5PLUGIN_CPP14``
     - Whether or not to compile C++14 code if available.
       Default: True if probed.
   * - ``HDF5PLUGIN_INTEL_IPP_DIR``
     - Experimental feature: Path to Intel `IPP`_ root directory.
       If provided, the LZ4 compression library and Blosc2 are set to use Intel `IPP`_.
       Default: Do not use INTEL `IPP`_ libraries.

Note: Boolean options are passed as ``True`` or ``False``.


.. _AVX2: https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2
.. _AVX512: https://en.wikipedia.org/wiki/AVX-512
.. _BMI2: https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set
.. _IPP: https://en.wikipedia.org/wiki/Integrated_Performance_Primitives
.. _SSE2: https://en.wikipedia.org/wiki/SSE2
.. _OpenMP: https://www.openmp.org/
