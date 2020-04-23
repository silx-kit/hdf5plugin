==============
 Contributing
==============

This project follows the standard open-source project github workflow, which is described in other projects like `matplotlib <https://matplotlib.org/devel/contributing.html#contributing-code>`_ or `scikit-image <https://scikit-image.org/docs/dev/contribute.html>`_.

Guidelines to add a compression filter
======================================

This briefly describes the steps to add a HDF5 compression filter to the zoo.

* Add the source of the HDF5 filter and compression algorithm code in a subdirectory in ``src/[filter]``.
  Best is to use ``git subtree`` else copy the files there (including the license file).
  A released version of the filter + compression library should be used.

  ``git subtree`` command::

    git subtree add --prefix=src/[filter]  [git repository]  [release tag] --squash

* Update ``setup.py`` to build the filter dynamic library by adding an extension using the ``HDF5PluginExtension`` class (a subclass of ``setuptools.Extension``) which adds extra files and compile options to enable dynamic loading of the filter.
  The name of the extension should be ``hdf5plugin.plugins.libh5<filter_name>``.

* Add a "CONSTANT" in ``hdf5plugins/__init__.py`` named with the ``FILTER_NAME`` which value is the HDF5 filter ID
  (See `HDF5 registered filters <https://portal.hdfgroup.org/display/support/Registered+Filters>`_).

* Add a ``"<filter_name>": <FILTER_ID_CONSTANT>`` entry in ``hdf5plugin.FILTERS``.
  You must use the same `filter_name` as in the extension in ``setup.py`` (without the ``libh5`` prefix) .
  The names in ``FILTERS`` are used to find the available filter libraries.

* In case of import errors related to HDF5-related undefined symbols, add eventual missing functions under ``src/hdf5_dl.c``.

* Add a compression options helper function named ``<filter_name>_options`` in ``hdf5plugins/__init__.py`` which should return a dictionary containing the values for the ``compression`` and ``compression_opts`` arguments of ``h5py.Group.create_dataset``.
  This is intended to ease the usage of ``compression_opts``.

* Add tests:

  - In ``test/test.py`` for testing reading a compressed file that was produced with another software.
  - In ``hdf5plugin/test.py`` for tests that writes data using the compression filter and the compression options helper function and reads back the data.

* Update the ``README.rst`` file to document:

  - The version of the HDF5 filter that is embedded in ``hdf5plugin``.
  - The license of the filter (by adding a link to the license file).
  - The ``hdf5plugin.<FILTER_NAME>`` filter ID "CONSTANT".
  - The ``hdf5plugin.<filter_name>_options`` compression helper function.

* Update ``doc/compression_opts.rst`` to document the format of ``compression_opts`` expected by the filter.

