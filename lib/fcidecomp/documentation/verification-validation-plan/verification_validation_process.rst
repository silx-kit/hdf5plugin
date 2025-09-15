.. _software_v&v_process_and_planning:

Software V&V Process Overview and Planning
------------------------------------------

Organization of V&V Activities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

V&V activities for the FCIDECOMP software are organized in the following phases:

-  the definition of a V&V plan (this document), which:

   -  identifies the goals and the scope of the activities;

   -  specifies the V&V strategy;

   -  defines the V&V methods;

   -  outlines the test specification design;

-  the definition of V&V specifications :ref:`[FCIDECOMP_TP] <[FCIDECOMP_TP]>`, in the form of:

   -  test cases, which aim at
      validating specific features or expected failures;

   -  test procedures specify for each test case the actions to be
      executed and the expected results;

-  the definition of the Verification Control Document :ref:`[FCIDECOMP_VCD] <[FCIDECOMP_VCD]>`, to detail, for each
   requirement in :ref:`[FCIDECOMP_WP] <[FCIDECOMP_WP]>`:

   -  the verification methods;

   -  verification notes, e.g. consulting with experts i.e. security and QA teams;

   -  the software version first implementing the requirement;

   -  the traceability between system requirements and the items verifying it
      (test cases, chapters of a document);

   -  test procedure automation flag (if applicable);

   -  notes;

   -  latest Test or Review-of-design Report;

   -  latest Report date;

   -  validated/verified flag;

-  the execution of the procedures and the recording of the results in a
   test report. This phase includes one or more on-factory (dry-run)
   test sessions, and is used for the official validation.

Test Plan
~~~~~~~~~

All the test procedures are performed upon completion of the source code and documentation
delivery milestone.

Schedule of the Verification and Validation Activities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The FCIDECOMP software source code is hosted at EUMETSAT GitLab instance
at https://gitlab.eumetsat.int/sepdssme/fcidecomp  .

V&V activities are executed as
detailed in the following table.


.. list-table:: Schedule of V&V activities.
    :header-rows: 1
    :widths: 60 25 15

    * - Activity
      - Event
      - Allocated
    * - Factory acceptance tests
      - 3 days before TRR
      - 2 hours
    * - Test readiness review
      - TRR
      - 1 hour
    * - On-site acceptance tests
      - OSAT
      - 2 hours
    * - Test review board
      - TRB
      - 1 hour

Anomaly reports mechanism
~~~~~~~~~~~~~~~~~~~~~~~~~~

Anomalies detected during the execution of tests
are tracked as issues in the FCIDECOMP GitLab instance,
with a target date for resolution.
Links to the issues are reported in the relevant test reports.

Responsibilities and Roles
~~~~~~~~~~~~~~~~~~~~~~~~~~

The following roles are required for completion of the V&V activities.
The :ref:`Table <table_responsibilities_and_roles>` outlines which individuals
from which organisations fulfil these roles.

.. _table_responsibilities_and_roles:

.. table:: Responsibilities and roles in V&V activities.
    :widths: 30 25 15 30

    +---------------------------+-----------------------+---------------------+--------------------------------------+
    | Activity                  | Resource              | Organisation        | Role                                 |
    +---------------------------+-----------------------+---------------------+--------------------------------------+
    | Factory acceptance        |  WP leader            |   B-Open            | Responsible for B-Open               |
    | test                      |  (M. Bottaccio)       |                     | validation activities.               |
    |                           +-----------------------+---------------------+--------------------------------------+
    |                           |  Support team         |   B-Open            | Execution of manual tests            |
    +---------------------------+-----------------------+---------------------+--------------------------------------+
    | Documentation review      | EUMETSAT Review Board |   EUMETSAT          | Documentation review,                |
    |                           |                       |                     | raising of RIDs                      |
    +---------------------------+-----------------------+---------------------+--------------------------------------+
    | On-site acceptance        |  WP Leader            |   B-Open            | Responsible for B-Open               |
    | tests                     |  (M. Bottaccio) or    |                     | validation activities                |
    |                           |  delegate             |                     |                                      |
    |                           +-----------------------+---------------------+--------------------------------------+
    |                           |  EUMETSAT Project     |   EUMETSAT          | Coordination of validation           |
    |                           |  Manager              |                     | activities.                          |
    +---------------------------+-----------------------+---------------------+--------------------------------------+


Other Resources
~~~~~~~~~~~~~~~~~

V&V activities also require the following resources to be setup in advance.
Test specifications (test cases) specify additional software pre-requisites which do not need to be
setup in advance.

.. _table_resources:

.. csv-table:: Other resources needed for V&V activities.
    :header: "Activity", "Resource", "Responsible organisation"

    "Execution of automated tests for verification", "GitLab instance with CI/CD active", "EUMETSAT"
    "Execution of automated tests for verification", "Linux machine with Docker_ installed", "EUMETSAT"
    "Execution of automated tests for verification", "GitLab Runner_ installed on the Linux machine provided by
    EUMETSAT", "B-Open"
    "Factory acceptance tests (manual)", "Reference machine described in :ref:`[FCIDECOMP_WP] <[FCIDECOMP_WP]>`
    with Docker, SSH access with Public IP", "EUMETSAT"
    "On-site acceptance tests", "Machines with the same requirements as for the FAT", "EUMETSAT"


.. note:: On-site acceptance tests can be executed on the same machine as in the FAT, under the assumption
    that the FCIDECOMP software is made available as read-only and the publishing server can be deployed as a Docker
    container.


.. _Docker: https://www.docker.com
.. _Runner: https://docs.gitlab.com/runner/
