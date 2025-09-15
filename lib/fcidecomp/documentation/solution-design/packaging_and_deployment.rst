.. _packaging_and_deployment:

Packaging and deployment
------------------------

Introduction
~~~~~~~~~~~~

This section describes the strategy to build and package the FCIDECOMP software in order to ensure
support for all the required systems, and how to deploy it on supported platforms.

.. _supported_platforms:

Supported platforms
~~~~~~~~~~~~~~~~~~~

The FCIDECOMP software supports the following platforms:

- Windows 10, 32 and 64 bit
- Ubuntu 18.04, Ubuntu 20.04 64 bit
- CentOS 7 64 bit

Details on the selection process leading to the list presented above are provided in
:ref:`a_design_justification`.

.. _conda_package:

Packaging as a Conda package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Packages are built using Conda, as it provides standardised environments with a large set of pre-compiled packages.
From the point of view of Conda, the operating systems listed in the :ref:`supported_platforms` paragraph can be
considered as two groups of OS: in Conda standardised environment it is enough to build the package for one Linux
distribution in order to make it compatible with other Linux distributions. So two Conda packages are released: one for
Linux distributions, and one for Windows 10.

As Conda is both a package manager and an environment manager, it allows to generate the same environment and install
the packages on different OSes of the same type and architecture (e.g. Linux 64-bit). For this reason its use ensures
long-term maintainability of the chosen solution, even in the case any OS listed in :ref:`supported_platforms` should
reach its end-of-life before the end of the period covered by MTG operations.

These Conda packages install both the FCIDECOMP libraries and its Python bindings. As a blueprint for the
Conda recipe, the :ref:`Conda recipe <[FCIDECOMP_CONDA]>` for the packaging of FCIDECOMP mantained by Martin Raspaud
from the Swedish Meteorological and Hydrological Institute has been used.

Conda packages are uploaded to EUMETSAT Anaconda repository https://anaconda.org/Eumetsat/repo.

.. _packaging_process:

Packaging process
=================

Three Conda packages are released: one for Linux and two for Windows (32-bit and 64-bit).

GitLab CI/CD pipelines to compile, build, test and upload the Conda packages to EUMETSAT Anaconda repository are
implemented. Two GitLab runners, i.e. applications dedicated to running jobs of a GitLab CI/CD pipeline, are deployed to
run the pipelines: one with a Docker executor on Linux and the other with a Shell executor on Windows 64-bit, which is
used to build both 64- and 32-bit packages.

See :ref:`a_runners` for details on the deployed GitLab runners.

.. _building_binaries:

Building the binaries from the source code
==========================================

The build system for the software binaries is drawn from the one used in the
:ref:`FCIDECOMP v1.0.2 source code <[FCIDECOMP_LATEST]>`, and adapted from there to guarantee support for all the
required systems. It uses ``GCC`` and ``MSVC`` to compile the binaries respectively in Linux and Windows systems.

Deploying on supported platforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The FCIDECOMP software is hosted on the public EUMETSAT Conda channel, and can be easily installed on supported
platforms using Conda.

Prerequisites
=============

Installation requires:

- ``conda``, installed as described at :ref:`[CONDA] <[CONDA]>`
- a connection to the Internet

Installation procedure
======================

The following instructions describe the FCIDECOMP software installation procedure, and are valid for any fo the
supported platforms:

#. Create a new Conda environment. Let's call it ``fcidecomp``, but any valid name would do (change the following
   instructions accordingly)::

    conda create -n fcidecomp python=$PYTHON_VERSION

   where Python versions currently supported by the FCIDECOMP software are 3.7 <= ``$PYTHON_VERSION`` <= 3.9.

#. Activate the environment::

    conda activate fcidecomp

#. Install the FCIDECOMP software and all its dependencies (see :ref:`installing_dependencies` for more details)::

    conda install -y -c anaconda -c conda-forge -c eumetsat fcidecomp


Once installation is complete, deactivate and reactivate the Conda environment to ensure the FCIDECOMP software is
correctly configured and ready to be used::

    conda deactivate
    conda activate fcidecomp


.. _installing_dependencies:

Installing dependencies
~~~~~~~~~~~~~~~~~~~~~~~

All :ref:`dependencies <dependencies>` are installed through Conda (see :ref:`conda_package`) except for Windows 32-bit
version of ``CharLS 2.1.0``: this dependency, in fact, is currently not present on any public Conda distribution
channel. It is thus compiled and installed, together with the FCIDECOMP software binaries, starting from the source code
available at its :ref:`GitHub repository <charls_v2>`.

In order to grant the ability to install the software even in case the remote repositories hosting its dependencies
should become unreachable, a separate assets repository is hosted on EUMETSAT infrastructure.
This assets repository hosts ``.tar.gz`` archives and Conda packages of all the dependencies needed for each release tag
of the FCIDECOMP software. For a possible more general solution, which is out of the scope of this project, see
:ref:`a_improvements`.

