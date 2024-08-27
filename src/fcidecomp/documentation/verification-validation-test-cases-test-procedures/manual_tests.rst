Manual Tests: Decompression via Java software and via the EUMETSAT Data Tailor software, and installation with offline dependencies repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section describes the test cases and procedures to validate:

- Opening and inspection of a JPEG-LS compressed MTG FCI L1C product via Panoply (reference Java software)
- Decompression of a JPEG-LS compressed MTG FCI L1C product via the EUMETSAT Data Tailor software
- Installation of the FCIDECOMP software without a connection to Internet (uses the offline dependencies
  repository :ref:`[FCIDECOMP_DEP_OFF] <[FCIDECOMP_DEP_OFF]>` hosted at EUMETSAT GitLab)

.. _test_data:

Tests reported in this section use the following MTG FCI L1C test data files :ref:`[TEST_DATA] <[TEST_DATA]>`:

.. list-table:: Test data
   :header-rows: 1
   :widths: 10 20 70

   * - ID
     - Description
     - File name
   * - TD.
       COMP.
       01
     - JPEG-LS compressed MTG FCI L1C body file
     - W_XX-EUMETSAT-Darmstadt_IMG+SAT_MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY--DIS-NC4E_C_EUMT_20200405000845
       _GTT_DEV_20200405000330_20200405000345_N_JLS_T_0001_0015.nc
   * - TD.DE
       COMP.
       01
     - Decompressed MTG FCI L1C body file
     - W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20200405000845
       _GTT_DEV_20200405000330_20200405000345_N__T_0001_0015.nc


.. _FCIDECOMP.JAVA.TC.01.01:

FCIDECOMP.JAVA.TC.01.01: decompression via Java software
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. list-table::
   :header-rows: 0
   :widths: 20 80

   * - Goal
     - Validate the ability to open JPEG-LS compressed MTG FCI L1C products with Panoply
   * - Input data
     - TD.COMP.01
   * - Pass/Fail criteria
     - Data contained in the TD.COMP.01 file can be plotted with Panoply


.. _FCIDECOMP.JAVA.TP.01.01:

FCIDECOMP.JAVA.TP.01.01: decompression via Java software
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
.. list-table::
   :header-rows: 1
   :widths: 10 55 35

   * - ID
     - Description
     - Expected output
   * - 0.
     - Configure Panoply to use the netCDF4-C library, following instructions reported in the README file hosted in the
       FCIDECOMP software GitLab repository :ref:`[FCIDECOMP] <[FCIDECOMP]>`
     - \-
   * - 1.
     - Open TD.COMP.01 in Panoply and create a plot using the ``/data/ir_105/measured/effective_radiance`` variable
     - Plot is generated


.. _FCIDECOMP.DT.TC.01.01:

FCIDECOMP.DT.TC.01.01: decompression via the EUMETSAT Data Tailor software
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. list-table::
   :header-rows: 0
   :widths: 20 80

   * - Goal
     - Validate the ability to decompress JPEG-LS compressed MTG FCI L1C products using the EUMETSAT Data Tailor
       software
   * - Input data
     - TD.COMP.01
   * - Pass/Fail criteria
     - The ``epct_plugin_mtg`` plugin is correctly installed

       The plugin successfully generates a decompressed netCDF output product


.. _FCIDECOMP.DT.TP.01.01:

FCIDECOMP.DT.TP.01.01: decompression via the EUMETSAT Data Tailor software
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
.. list-table::
   :header-rows: 1
   :widths: 5 50 45

   * - ID
     - Description
     - Expected output

   * - 0.
     - Install the ``epct`` and ``epct_plugin_mtg`` packages as described in the README file hosted in the
       ``epct_plugin_mtg`` software GitLab repository :ref:`[EPCT_MTG] <[EPCT_MTG]>`

       Check the ``epct_plugin_mtg`` si correctly installed, running the command:

       ``epct info``
     - The output of the command reports ``epct_mtg`` under the ``registered_backends`` key

   * - 1.
     - Decompress the TD.COMP.01 file. To do so, open a ``python`` terminal and run the following lines:
       .. code-block::

       >>> from epct import api
       >>> chain_config = {"product": "MTGFCIL1", "format": "netcdf4_satellite"}
       >>> api.run_chain(["$COMPRESSED_PRODUCT"], chain_config=chain_config, target_dir="$OUTPUT_DIR")

       where

       * ``$OUTPUT_DIR`` is the path to the directory where the decompressed file will be written,
         which should be different from the directory containing TD.COMP.01
       * ``$COMPRESSED_PRODUCT`` is the path to TD.COMP.01
     - The output of the command reports

       ``*** STOP PROCESSING - Status DONE ***``

   * - 2.
     - Check that the output product is actually decompressed, running in the terminal the command:

       ``ncdump -h -s $OUTPUT_PRODUCT | grep _Filter``

       where ``$OUTPUT_PRODUCT`` is the path to the decompressed output product, reported in the printed output of
       step 1
     - The output of the command is empty

   * - 3.
     - Check that the output product has been correctly decompressed, by opening it with Panoply (after reverting step 0
       of `FCIDECOMP.JAVA.TP.01.01`_) and creating a plot using the ``/data/ir_105/measured/effective_radiance``
       variable
     - Plot is generated


.. _FCIDECOMP.OFF.TC.01.01:

FCIDECOMP.OFF.TC.01.01: installation of the FCIDECOMP software using the offline dependencies repository
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. list-table::
   :header-rows: 0
   :widths: 20 80

   * - Goal
     - Validate the possibility to install the FCIDECOMP software using the offline dependencies repository
       :ref:`[FCIDECOMP_DEP_OFF] <[FCIDECOMP_DEP_OFF]>`
   * - Input data
     - TD.COMP.01
   * - Pass/Fail criteria
     - The FCIDECOMP software is correctly installed

       The plugin can be used to successfully generates a decompressed netCDF output product


.. _FCIDECOMP.OFF.TP.01.01:

FCIDECOMP.OFF.TP.01.01: installation of the FCIDECOMP software using the offline dependencies repository
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

.. list-table::
   :header-rows: 1
   :widths: 10 55 35

   * - ID
     - Description
     - Expected output
   * - 0.
     - Install the FCIDECOMP software following instructions reported in the README file at :ref:`[FCIDECOMP_DEP_OFF]
       <[FCIDECOMP_DEP_OFF]>`, at the "Installing the EUMETSAT FCIDECOMP software using the local Conda channel" >
       "Installation" > "Installation from a local FCIDECOMP Conda package". In those instructions,
       ``$PYTHON_VERSION`` could be limited to ``3.9``
     - The installation completes without errors
   * - 1.
     - Deactivate and re-activate the ``conda`` environment in which the FCIDECOMP software has been installed, and run
       the following command:

       ``nccopy -F none $TO_TD.COMP.01_PATH $DECOMPRESSED_OUTPUT_PATH``

       where:

       * ``$TD.COMP.01_PATH`` is the path to the TD.COMP.01 test file
       * ``$DECOMPRESSED_OUTPUT_PATH`` is the path to the netCDF file where the decompressed output dataset will be
         saved
     - The command runs without errors
   * - 2.
     - Check that the output product has been correctly decompressed, by opening it with Panoply (after reverting step 0
       of `FCIDECOMP.JAVA.TP.01.01`_) and creating a plot using the ``/data/ir_105/measured/effective_radiance``
     - Plot is generated


