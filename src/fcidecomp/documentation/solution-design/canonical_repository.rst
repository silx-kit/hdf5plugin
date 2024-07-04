.. _creation_of_canonical_repository:

Creation of canonical repository
--------------------------------

Introduction
~~~~~~~~~~~~

A canonical repository is established on the EUMETSAT GitLab service at https://gitlab.eumetsat.int/sepdssme/fcidecomp
for development purposes. Each time a new release is produced, the corresponding code is synchronized to the public
EUMETSAT Open Source repository at https://gitlab.eumetsat.int/open-source.

.. _repository_initialization:

Repository initialization
~~~~~~~~~~~~~~~~~~~~~~~~~

The last version of FCIDECOMP released before the establishment of the repository, :ref:`FCIDECOMP v1.0.2 <[FCIDECOMP_LATEST]>`, 
is used to initialise the repository.

A new minor release adding README, BUILD, INSTALL, and LICENCE files, starting the Changelog, codifying the use of
semantic versioning for future versions and adding a standardised build system is published. In this phase, the
FCIDECOMP source code is put in a dedicated ``src`` directory at the top-level of the repository.

Test approach
~~~~~~~~~~~~~

The code in the new repository is initially tested using a small set of automated tests and test data,
following the V&V strategy defined in the :ref:`verification and validation plan <[VV_PLAN]>`.