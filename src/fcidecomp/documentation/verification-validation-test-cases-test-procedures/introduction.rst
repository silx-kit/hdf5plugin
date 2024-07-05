.. _introduction:

Introduction
------------

Purpose
~~~~~~~

The document details the V&V Test Cases and Test Procedures
for the Work Package 'Support to CharLS decompression for MTG users'
in the context of the EUMETSAT Data Tailor Web Service.

Scope
~~~~~

Objective of this document is indicate the procedures to validate
the ability of the FCIDECOMP software to decompress FCI L1c NRT data,
meeting the goals defined in the :ref:`Verification & Validation Plan <[FCIDECOMP_VV]>`.

These aim to verify a working installation of the EUMETSAT FCIDECOMP software and
its ability to decompress FCI L1c NRT data via selected CLI tools, Python,
the EUMETSAT Data Tailor Software and selected Java Software.

It is addressed at:

-  developers/quality manager, to support them in the internal V&V process,

-  EUMETSAT officers in charge of the validation of the software.


Applicable Documents
~~~~~~~~~~~~~~~~~~~~

.. list-table:: Applicable documents.
  :header-rows: 1
  :widths: 25 35 40

  * - #
    - Title
    - Reference

  * - [FCIDECOMP_VCD]

      .. _[FCIDECOMP_VCD]:

    - EUMETSAT Data Tailor Web Service - Support to CharLS decompression for MTG users - Verification Control Document
    - `fcidecomp/documentation/verification-validation-plan-test-case-procedures/FCIDECOMP_VCD.xlsx <../../../verification-validation-test-cases-test-procedures/FCIDECOMP_VCD.xlsx>`_

  * - [FCIDECOMP_VV]

      .. _[FCIDECOMP_VV]:

    - EUMETSAT Data Tailor Web Service - Support to CharLS decompression for MTG users - Verification & Validation Plan
    - `fcidecomp/documentation/verification-validation-plan <../../../verification-validation-plan/_build/html/index.html>`_

  * - [FCIDECOMP_WP]

      .. _[FCIDECOMP_WP]:

    - Support to CharLS decompression for MTG users - Work package description
    - EUM/SEP/WPD/21/1244304


Reference Documents
~~~~~~~~~~~~~~~~~~~

.. list-table:: Reference documents.
  :header-rows: 1
  :class: longtable
  :widths: 23 42 35

  * - #
    - Title
    - Reference

  * - [EPCT_MTG]

      .. _[EPCT_MTG]:

    - ``epct_plugin_mtg`` software GitLab repository
    - `https://gitlab.eumetsat.int/data-tailor/epct_plugin_mtg/-/blob/development/ <https://gitlab.eumetsat.int/data-tailor/epct_plugin_mtg/-/blob/development/>`_

  * - [FCIDECOMP]

      .. _[FCIDECOMP]:

    - FCIDECOMP software GitLab repository
    - `https://gitlab.eumetsat.int/sepdssme/fcidecomp/fcidecomp/-/tree/development <https://gitlab.eumetsat.int/sepdssme/fcidecomp/fcidecomp/-/tree/development>`_


  * - [FCIDECOMP_DEP_OFF]

      .. _[FCIDECOMP_DEP_OFF]:

    - FCIDECOMP Softwre offline dependencies repository
    - `https://gitlab.eumetsat.int/sepdssme/fcidecomp/offline-dependencies/-/tree/development <https://gitlab.eumetsat.int/sepdssme/fcidecomp/offline-dependencies/-/tree/development>`_


  * - [FCIDECOMP_PIPELINES]

      .. _[FCIDECOMP_PIPELINES]:

    - FCIDECOMP project's CI/CD pipelines
    - `https://gitlab.eumetsat.int/sepdssme/fcidecomp/fcidecomp/-/pipelines <https://gitlab.eumetsat.int/sepdssme/fcidecomp/fcidecomp/-/pipelines>`_


  * - [TEST_DATA]

      .. _[TEST_DATA]:

    - MTG FCI L1C test data
    - `https://gitlab.eumetsat.int/data-tailor/epct-test-data/-/tree/development/MTG/MTGFCIL1 <https://gitlab.eumetsat.int/data-tailor/epct-test-data/-/tree/development/MTG/MTGFCIL1>`_


