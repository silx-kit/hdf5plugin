Organization of Test Cases, Non-regression Tests
------------------------------------------------

Organization of Test Cases and Order of Execution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Automatic test cases (FCIDECOMP.CLI.TC.01.01 and FCIDECOMP.PY.TC.01.01) test
the FCIDECOMP software can be used to decompress FCI L1c NRT data
via command line tools and via Python.
The netCDF4 ``nccopy`` tool is taken as reference for command line tools,
and the ``netCDF4`` Python library is taken as reference for use via Python.

Manual test cases test the FCIDECOMP software
can be used to decompress FCI L1c NRT data
via selected Java Software and via the Data Tailor Software (FICDECOMP.JAVA.TC.01.01 and FCIDECOMP.DT.TC.01.01),
and that it can be installed using the offline dependencies repository
:ref:`[FCIDECOMP_DEP_OFF] <[FCIDECOMP_DEP_OFF]>` (FCIDECOMP.OFF.TC.01.01).
``Panoply`` is taken as reference for decompression via Java Software

.. _organization_of_test_cases_conventions:

Conventions
~~~~~~~~~~~

The following conventions have been adopted in writing the Test
Procedures:

-  all paths follow the Linux convention (“/” as the separator)

-  commands executed in a terminal are written with a BASH syntax (e.g.
   newlines).

For validation purposes, reference platform are:

- Centos 7 64-bit, Windows 10 64-bit and Windows 10 32-bit for FCIDECOMP.CLI.TC.01.01 and FCIDECOMP.PY.TC.01.01
- Ubuntu 20.04 LTS 64-bit for FCIDECOMP.JAVA.TC.01.01
- Ubuntu 20.04 LTS 64-bit and Windows 10 64-bit for FCIDECOMP.DT.TC.01.0 and FCIDECOMP.OFF.TC.01.01


.. _reference_platform:

Reference Platform and Execution Time for Test Cases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As a support to the planning of test sessions, the following table
summarizes hardware and software pre-requisites and average execution
times for each test case category, for the reference platforms with the
following specifications:

- Centos 7 64-bit:

    - Docker container running on a machine with the following specifications:

        - Processor: Intel(R) Xeon(R) Gold 6252 CPU @ 2.1 GHz, 1 Core

        - RAM: 51 GB, 4 GB required (estimate)

        - Disk space: 4 GB required

- Ubuntu 20.04 LTS 64-bit:

    - Processor: Intel® Core™ i7-10510U CPU @ 1.80GHz × 8 (2 cores, 4 threads)

    - RAM: 16 GB, 4 GB required (estimate)

    - Disk space: 4 GB required

- Windows 10 64-bit/32-bit:

    - Processor: Intel(R) Xeon(R) Gold 6132 CPU @ 2.60GHz (2 processors)

    - RAM: 32 GB installed, 4 GB required (estimate)

    - Disk space: 4 GB required

.. list-table:: Estimated execution times for test cases under each test sub-category.
   :header-rows: 1
   :widths: 20 40 40
   :class: longtable

   * - Category ID
     - Software pre-requisites
     - Average execution time in minutes

   * - :ref:`FCIDECOMP.CLI.TC.01.01 <FCIDECOMP.CLI.TC.01.01>`

     - - OS: Centos 7 64-bit, Windows 10 64-bit and Windows 10 32-bit
       - ``conda`` environment with

         * ``fcidecomp`` installed, as described in the INSTALL file hosted in the FCIDECOMP software GitLab repository
           :ref:`[FCIDECOMP] <[FCIDECOMP]>`
         * ``pytest`` installed, as described at `<https://anaconda.org/anaconda/pytest>`_

       :Note: all requirements are satisfied by the GitLab CI/CD environment in which the test is run.
     - < 1 min

   * - :ref:`FCIDECOMP.PY.TC.01.01 <FCIDECOMP.PY.TC.01.01>`

     - - OS: Centos 7 64-bit, Windows 10 64-bit and Windows 10 32-bit
       - ``conda`` environment with

         * ``fcidecomp`` installed, as described in the INSTALL file hosted in the FCIDECOMP software GitLab repository
           :ref:`[FCIDECOMP] <[FCIDECOMP]>`
         * ``pytest`` installed, as described at `<https://anaconda.org/anaconda/pytest>`_

       :Note: all requirements are satisfied by the GitLab CI/CD environment in which the test is run.
     - < 1 min

   * - :ref:`FCIDECOMP.JAVA.TC.01.01 <FCIDECOMP.JAVA.TC.01.01>`

     - - OS: Ubuntu 20.04 LTS 64-bit
       - ``conda`` environment with ``fcidecomp`` installed, as described in the INSTALL file hosted in the FCIDECOMP
         software GitLab repository :ref:`[FCIDECOMP] <[FCIDECOMP]>`
       - ``panoply`` installed, as described at `<https://www.giss.nasa.gov/tools/panoply/download/>`_
     - 15 min

   * - :ref:`FCIDECOMP.DT.TC.01.01 <FCIDECOMP.DT.TC.01.01>`

     - - OS: Ubuntu 20.04 LTS 64-bit and Windows 10 64-bit
       - ``conda`` environment with ``epct`` and ``epct_plugin_mtg`` installed, as described in the README file hosted
         in the ``epct_plugin_mtg`` GitLab repository :ref:`[EPCT_MTG] <[EPCT_MTG]>`
       - ``panoply`` installed, as described at `<https://www.giss.nasa.gov/tools/panoply/download/>`_
     - 15 min

   * - :ref:`FCIDECOMP.OFF.TC.01.01 <FCIDECOMP.OFF.TC.01.01>`

     - - OS: Ubuntu 20.04 LTS 64-bit and Windows 10 64-bit
       - ``conda`` installed, as described at
         `<https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html>`_
       - ``panoply`` installed, as described at `<https://www.giss.nasa.gov/tools/panoply/download/>`_
     - 15 min


Requirements and Validation Goals addressed by Test Cases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following table maps the requirements detailed in the Work Package description
:ref:`[FCIDECOMP_WP] <[FCIDECOMP_WP]>` with the validation goals defined in the
FCIDECOMP Verification and Validation Plan:ref:`[FCIDECOMP_VV] <[FCIDECOMP_VV]>`
and respective tests to meet these goals defined herein:

.. list-table:: Requirements and validation goals addressed by the test cases defined in this document.
   :header-rows: 1
   :widths: 10 40 50

   * - Requirement
     - Validation Goal
     - Corresponding Tests (IDs)

   * - DTWS-FCI-010
     - VG1, VG2, VG3, VG4
     - All

   * - DTWS-FCI-020
     - VG4
     - FCIDECOMP.DT.TC.01.01

   * - DTWS-FCI-030
     - VG1, VG3
     - FCIDECOMP.CLI.TC.01.01, FCIDECOMP.PY.TC.01.01, FCIDECOMP.JAVA.TC.01.01

   * - DTWS-FCI-040
     - VG5, VG6
     - FCIDECOMP.OFF.TC.01.01

   * - DTWS-FCI-050
     - Contractual requirement
     - No test expected



