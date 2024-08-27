.. _automatic_tests:

Automatic Tests: Decompression via Command Line Tools and via Python
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section presents the automatic validation tests for the FCIDECOMP software.
These tests are run on the project's CI/CD pipelines :ref:`[FCIDECOMP_PIPELINES] <[FCIDECOMP_PIPELINES]>` on EUMETSAT's
GitLab instance,
on machines provided by EUMETSAT with CentOS 7 64-bit (run in Docker container), Windows 10 64-bit and
Windows 10 32-bit,
with all requirements described in ":ref:`reference_platform`" satisfied.

The tests can also be run on a local machine, provided:

- a clone of the FCIDECOMP software GitLab repository :ref:`[FCIDECOMP] <[FCIDECOMP]>` is present on the machine
- MTG FCI L1C test data :ref:`[TEST_DATA] <[TEST_DATA]>` are present on the machine at the local path
  ``$EPCT_TEST_DATA_DIR/MTG/MTGFCIL1``, where ``$EPCT_TEST_DATA_DIR`` is an arbitrary local path
- The environment variable ``EPCT_TEST_DATA`` is set equal to the path ``$EPCT_TEST_DATA_DIR``
- pre-requisites described in ":ref:`reference_platform`" are satisfied

Once the above requirements are satisfied, automatic tests can be run executing the following command from the root
directory of the FICDECOMP software repository:

``pytest -vv``

:Note: The test case descriptions of the automated tests are extracted from the test files themselves.

.. _FCIDECOMP.CLI.TC.01.01:

FCIDECOMP.CLI.TC.01.01: decompression via command line tools
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. automodule:: test_CLI_TC_01_01
    :members:
    :undoc-members:
    :show-inheritance:

.. _FCIDECOMP.PY.TC.01.01:

FCIDECOMP.PY.TC.01.01: decompression via Python
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. automodule:: test_PY_TC_01_01
    :members:
    :undoc-members:
    :show-inheritance:
