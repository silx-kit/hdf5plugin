.. _test_specification_design:

Test Specification Design
~~~~~~~~~~~~~~~~~~~~~~~~~

Test Cases and Test Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Verification specifications in this document are designed at two levels:
test cases and test procedures.

**Test cases** are self-consistent test descriptions, which aim
at validating specific features or expected failures. In the design of
test cases, the following guidelines are considered:

-  streamline execution, i.e. avoid as much as possible the repetition
   of procedure steps across different test procedures;

-  validate the normal flow of operations before (dead-end) branches and
   response to errors.

Test cases are grouped in categories related to the feature, product or behaviour they
validate.

Each test case describes:

-  the input test data;

-  the reference output data, if applicable;

-  the pass/fail criteria the test case must satisfy to be considered as
   passed;

-  the specific test environment.

A **test procedure** contains the sequential list of operations
and the corresponding expected outputs, required to validate a
test case.

Test cases and test procedures are described in the :ref:`Verification & Verification Test Cases and Test Procedures
document <[FCIDECOMP_TP]>`, which also contains the 
traceability between the test cases and corresponding verification goals
validate.

Traceability between test cases and test procedures is
ensured by the naming convention described in the following paragraph.

The automated tests are pytest files with in-code documentation; with this approach,
the documentation is written in such a way that it describes the test case, and the
test code is the test procedure. The part of the automated tests in the
Test Cases and Procedures document will be then automatically generated from the code.

Identification of Test Cases and Test Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Test cases and test procedures are identified as follows.

- a test case is identified as FCIDECOMP.xxxx.TC.yy.zz, where:
  - `xxxx` is a category identifier (up to four characters)
  - `yy` is a two-digit progressive number within each category, to group test cases in homogeneous subcategories
  - zz being a two-digit progressive identifier within the subcategory.

- a test procedure for the test case FCIDECOMP.xxxx.TC.yy.zz is identified as FCIDECOMP.xxxx.TP.yy.zz.

Category identifiers are introduced in order to organize test cases in groups which may have e.g. common
pre-requisites, and to improve clarity and maintenance of the V&V plan.

The following categories are defined:

:CLI:
    Tests for the decompression of FCI L1c NRT data via a command line interface (VG1)

:PY:
    Tests for the decompression of FCI L1c NRT data via Python (VG2)

:JAVA:
    Tests for the decompression of FCI L1c NRT data via Java (VG3)

:DT:
    Tests for the decompression of FCI L1c NRT data via the Data Tailor (VG4)

:OFF:
    Tests for the installation of the FCIDECOMP Software using the offline dependencies repository hosted at EUMETSAT
    Gitlab (VG5, VG6)
