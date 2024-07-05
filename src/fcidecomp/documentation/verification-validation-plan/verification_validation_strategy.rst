.. _v&v_strategy:

V&V Strategy
-------------

Introduction
^^^^^^^^^^^^

This section describes the V&V strategies devised to meet the goals. V&V methods referred in the
text are described in section :ref:`v&v_methods`.

As a general rule, for both validation and verification, tests are the method of choice, if applicable,
and automated tests are preferred to manual tests.



Decompression of FCI L1c NRT data via command line (VG1)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Decompression of FCI L1c NRT data via command line is automatically tested using

Decompression of FCI L1c NRT data via Python (VG2)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Decompression of FCI L1c NRT data via Python is automatically tested.

Decompression of FCI L1c NRT data via Java (VG3)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Decompression of FCI L1c NRT data via Java is tested manually, by demostrating that
MTG FCI L1C files can be opened with `Panoply <https://www.giss.nasa.gov/tools/panoply/>`_.

Decompression of FCI L1c NRT data via the Data Tailor software (VG4)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Decompression of FCI L1c NRT data via the Data Tailor software is tested manually.

Since the Data Tailor supports MTG through plugins, decompression of FCI L1c NRT data via the Data Tailor can be
ensured by ensuring the plugin can be installed and can customise one compressed FCI L1c product for each Data Tailor
release.

Package licensed as free and open source software (VG5)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The license of the FCIDECOMP software is verified by code inspection.


Tests for VG1 and VG2 are executed on all the applicable platforms; tests for VG3 
are only executed on one Linux distribution, as support on Java depends on 
the same requirements of VG1 and the integration strategy in the selected Java tools 
is the same on all platforms.

Tests for VG4 are only executed on one Linux distribution, as the integration in the 
Data Tailor relies on the Python support validated with VG2.




All test procedures are organized and designed to test functionalities so that they are reasonably
achieved on all the applicable platforms (MTG user stations that match the approved baseline). In case of
Python (VG2) and Java (VG3) the test procedures are actually executed to a restricted set of platforms,
delegating the requirement matching to the cross-platform nature of these programming languages.
