Installing the EUMETSAT FCIDECOMP software
------------------------------------------

Installation procedures have been tested for the following Operating Systems:

- Linux CentOS 7 64-bit
- Linux Ubuntu 18.04 LTS 64-bit
- Linux Ubuntu 20.04 LTS 64-bit
- Windows 10 64-bit
- Windows 10 32-bit

There are four ways to install the EUMETSAT FCIDECOMP software:

- from the Conda packages hosted in the EUMETSAT Anaconda repository (requires an Internet connection),
  as described in the :ref:`install_anaconda_repo` section;
- using the provided Conda recipe, as described in the :ref:`install_conda_recipe` section;
- from the Conda packages downloaded as artifacts of a CI/CD GitLab pipeline on the target machine
  (mostly for testing purposes), as described in the :ref:`install_artifacts` section;
- using the FCIDECOMP dependencies repository hosted at :ref:`[FCI_DEP_REPOSITORY] <[FCI_DEP_REPOSITORY]>`,
  as described in the :ref:`install_dep_repo` section.

.. _install_anaconda_repo:

Installation from the Anaconda repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For detailed instructions on how to install the FCIDECOMP software from the public EUMETSAT Anaconda repoistory, see
the INSTALL file in the root directory of the public FCIDECOMP GitLab repository at
:ref:`[EUM_PUB_REPOSITORY] <[EUM_PUB_REPOSITORY]>`.

.. _install_conda_recipe:

Installation using the Conda recipe
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For detailed instructions on how to install the FCIDECOMP software using its Conda recipe, see
the INSTALL file in the development FCIDECOMP GitLab repository ``/conda`` directory at
:ref:`[FCI_DEV_REPOSITORY] <[FCI_DEV_REPOSITORY]>`.

.. _install_artifacts:

Installation using an 'artifacts' file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pre-requisites
^^^^^^^^^^^^^^

Installation requires:

- the FCIDECOMP Conda packages, downloaded as a single ``zip`` artifacts file from the "deploy" job of the project's
  CI/CD pipeline for the branch/tag of interest;
- ``conda``, installed as described at
  :ref:`[CONDA_INST_INSTR] <[CONDA_INST_INSTR]>`.

Installation
^^^^^^^^^^^^

Start by creating a new Conda environment. Let's call it ``fcidecomp``, but any valid name would do (change the
following instructions accordingly)::

    conda create -n fcidecomp python=$PYTHON_VERSION

where Python versions currently supported by the FCIDECOMP software are 3.7 <= ``$PYTHON_VERSION`` <= 3.9.

Activate the environment::

    conda activate fcidecomp

Unzip the FCIDECOMP Conda packages. They end up in a directory which ends with ``conda-channel``.
Execute (replace ``$CONDA_CHANNEL_PATH`` with the path to the directory, including ``conda-channel``)::

    conda install -y -c anaconda -c conda-forge -c $CONDA_CHANNEL_PATH fcidecomp

.. _install_dep_repo:

Installation using the FCIDECOMP dependencies repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An archive of the dependencies needed to install the FCIDECOMP software is hosted at
:ref:`[FCI_DEP_REPOSITORY] <[FCI_DEP_REPOSITORY]>`, in case any of the required dependencies should be
unavailable from public channels. For detailed instructions on how to install the FCIDECOMP software using this
repository, see the README file in the FCIDECOMP dependencies repository root directory at
:ref:`[FCI_DEP_REPOSITORY] <[FCI_DEP_REPOSITORY]>`.

Post-installation configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the installation has completed, re-activate the Conda environment running the following commands::

    conda deactivate
    conda activate fcidecomp

This last step ensures the ``HDF5_PLUGIN_PATH`` environment variable is correctly set to the directory containing the
FCIDECOMP decompression libraries (check if that's actually the case to ensure the FCIDECOMP is correctly configured
and ready to be used).