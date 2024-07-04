.. _v&v_methods:

Verification and Validation Methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following V&V methods are envisioned for the project:

-  *Inspection* (I): method using visual determination of
   physical characteristics (e.g., inspecting outputs with visualization tools).

-  *Analysis (A)*: method using analytical data or simulations under
   defined conditions to show theoretical compliance.

-  *Review of Design* (D): method using approved records or
   evidence that unambiguously show that the requirement is met, e.g.
   design documents.

-  *Test* (T): An action by which the operability, supportability, or
   performance capability of an item is verified and validated when subjected to
   controlled conditions that are real or simulated.


Review of Design Execution and Reporting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Review of design is executed during documentation reviews.
Anomalies are recorded as RIDs, which are tracked as issued in the project
space on EUMETSAT GitLab.

The successful closure of all the RIDs corresponds to the success of the verification
activity, which is then recorded in a minute of meeting.


Inspection and Test Execution and Reporting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Planning
""""""""

Requirements in :ref:`[FCIDECOMP_WP] <[FCIDECOMP_WP]>` which are verified by test
are addressed during a dedicated test campaign
which typically follows the TRR/TRB process described in :ref:`[OPSTRR] <[OPSTRR]>`.

The full set of tests to meet the goals in section :ref:`v&v_goals` will be
delivered with the source code and documentation delivery project milestone.

Two execution approaches are envisioned: automatic and manual.


Automated Tests
"""""""""""""""

Automated tests provide a repeatable and fast verification tool
to verify compliance of the FCIDECOMP software against the requirements.

The `pytest` testing framework is used to write and execute automated tests.

Such tests are executed in the `test` step of continuous integration pipelines triggered from
within the project GitLab repository. This ensures testing repeatability.

Automated tests will be executed on a set of test data, one per each type of input products,
stored in a location reachable from the pipelines.

The following table lists the test types, their scope and the event which triggers them.

.. list-table:: Automated test types and triggers.
    :header-rows: 1
    :widths: 10 15 14 11 50

    *   - Type
        - Scope
        - Position in the source code
        - Event
        - Notes
    *   - Low-level unit tests
        - Used internally by the development team to validate integration of the Data Tailor plugin.
          Also used to measure test coverage.
        - :code:`tests` in each package root folder
        - At each commit into the repository
        - \-
    *   - Validation tests
        - Tests used for the validation of the FCIDECOMP software.
        - In the root folder of the FCIDECOMP source code, in folder :code:`validation-tests`
        - At each code revision tag
        - The folder contains one file for each test procedure.
          Files are named :code:`test_<category>_TP_<subgroup>_<progressive_id>`,
          to allow the traceability to the corresponding test case (see
          :ref:`test_specification_design` for test case identification).
          Validation tests need the test data package to be downloaded on the runner machine.

The reports for Validation tests are accessed directly in GitLab, in the section "CI/CD->Pipelines",
clicking on the pipeline identifier, then on the "Tests" section.

They can be downloaded as Junit XML files as
artifacts from the "Artifacts" section of the test job ("CI/CD->Pipelines",
clicking on the pipeline identifier, then on "Jobs"), to be attached to test reports if needed.

Automated tests are used for the V&V of:

- ability to use the FCIDECOMP software to decompress FCI L1c NRT data via command line (VG1)
- ability to use the FCIDECOMP software to decompress FCI L1c NRT data via Python (VG2)

Individual automated tests can also be launched manually from the command line.
This allows in particular to access the
generated products, to validate them manually.


Manual Validation Tests and Inspection
""""""""""""""""""""""""""""""""""""""

Manual tests are conducted by executing the test procedure steps in a test case
in sequence, verifying the resulting behaviour with respect to the expected one for each step.

Inspection of the system or of the test results is also included in some test steps.

The outcome of each test is written in the test report.

Manual validation tests need the validation test data package.

Manual tests for the validation of:

- ability to use the FCIDECOMP software to decompress FCI L1c NRT data in Java programs (VG3)
- ability to use the FCIDECOMP software to decompress FCI L1c NRT data via the Data Tailor Software (VG4).


Test Outcomes
^^^^^^^^^^^^^

The possible outcomes of a test procedure are classified as follows:

-  *passed*: the outcome of the test or of a procedure step conforms to
   the expected result.

-  *passed with limitations*:

   -  either the outcome conforms to the expected result, but some
      relevant observation has been made;

   -  or there is a minor discrepancy between expected and observed
      outcome, which does not however invalidate the test.

-  *failed*: the test or the test step have not produced the expected
   behaviour, and the discrepancy is significant. The test is considered
   "failed". However, the failure is not such to block the execution of
   the remaining tests.

-  *critically failed*: the test or the procedure step have failed; the
   test is considered failed, and failure is such to block the execution
   of the remaining tests.


Test Reports
^^^^^^^^^^^^^

A **test report** will detail:

-  the date of the execution of the tests

-  participants

-  software version under test

-  the objective of the tests

-  the overall outcome of the tests

-  for each test, its outcome and major observations resulting
   from the execution, if any

-  optionally, notes and comments.
