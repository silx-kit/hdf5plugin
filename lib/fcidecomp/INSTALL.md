# Install the EUMETSAT FCIDECOMP software

This document describes how to install the EUMETSAT FCIDECOMP software.

Two installation methods are available:

- [installation via ``conda``](#installation-from-conda-package) using pre-built packages,
  supported for the following Operating Systems:
  - Linux CentOS 7 64-bit
  - Linux Ubuntu 18.04 LTS 64-bit
  - Linux Ubuntu 20.04 LTS 64-bit
  - Windows 10 64-bit
  - Windows 10 32-bit.
- build and installation [from the source code](#build-and-installation-from-the-source-code),
  available for the following operating systems
  - RockLinux 8 64-bit
  - Ubuntu Linux 20.04 64-bit

## Installation from `conda` package

### Pre-requisites

Installation requires:

- `conda`, installed as described
  [here](<https://conda.io/projects/conda/en/latest/user-guide/install/index.html>)
- a connection to the Internet

### Installation

Start by creating a new `conda` environment. Let's call it `fcidecomp`, but
any valid name would do (change the following instructions accordingly):

    conda create -n fcidecomp python=$PYTHON_VERSION
    
where Python versions currently supported by ``fcidecomp`` are 3.7 <= `$PYTHON_VERSION` <= 3.9.

Activate the environment:

    conda activate fcidecomp

Now execute:

    conda install -y -c anaconda -c conda-forge -c eumetsat fcidecomp

### Post-installation configuration

Once the installation has completed, re-activate the `conda` environment running the following commands:

    conda deactivate
    conda activate fcidecomp
    
This last step ensures the `HDF5_PLUGIN_PATH` environment variable is correctly set to the directory containing the
FCIDECOMP decompression libraries.


## Build and installation from the source code

### Install pre-requisite packages

#### Rockylinux 8 64-bit

Instructions in this section need to be executed as super-user. Versions are pinned
for clarity and replicability (other versions may work as well).

First, install the software required to build the binaries (common for ``CharLS`` and ``fcidecomp``):

    yum install -y git
    yum install -y cmake-3.20.2 gcc-c++-8.5.0 # cmake 3.20.2 also installs make 4.2.1;  gcc-c++ also installs gcc

Then, let's install some ``fcidecomp``-specific dependency packages; ``powertools`` from
the ``epel-release`` repository is needed as well, so execute the following:

    yum install -y zlib-devel-1.2.11
    dnf install -y epel-release
    dnf config-manager --set-enabled powertools
    yum install -y hdf5-devel-1.10.5


#### Ubuntu Linux 20.04 64-bit

First, install the software required to build the binaries (common for ``CharLS`` and ``fcidecomp``):

    sudo apt install -y git 
    sudo apt-get install -y cmake gcc=4:9.3.0-1ubuntu2 g++ 

Then, let's install some ``fcidecomp``-specific dependency packages:

    sudo apt install -y zlib1g-dev libhdf5-dev

  
### Build CharLS

Next step is to build and install CharLS. `fcidecomp` has been tested with CharLS version ``2.1.0``, so let's
use this one:

    git clone -b 2.1.0 https://github.com/team-charls/charls.git && cd charls 
    mkdir release && cd release
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=On ..
    make && sudo make install

### Build and install ``fcidecomp``

Now we can build ``fcidecomp`` proper.

Obtain the source code either from the Open Source EUMETSAT repository (set ``FCIDECOMP_TAG`` to the proper 
``fcidecomp`` tag, e.g. `2.0.0`):

     git clone -b $FCIDECOMP_TAG https://gitlab.eumetsat.int/open-source/fcidecomp/

or obtain the source code as a `tar.gz` package and uncompress it:

     tar xzvf fcidecomp-$FCIDECOMP_TAG.tar.gz


Now build and install the software as follows (set ``FCIDECOMP_TAG`` to the proper ``fcidecomp`` tag,
``PATH_TO_CHARLS`` to the path where CharLS has been installed, i.e. ``/usr/local/lib`` if the defaults
are used, and change the installation paths below as deemed appropriate. Note that
install commands require super-user privileges):

    cd fcidecomp/src/fcidecomp
    ./gen/build.sh fcicomp-jpegls release -DCMAKE_PREFIX_PATH=$PATH_TO_CHARLS   -DCMAKE_INSTALL_PREFIX=/usr/local/fcidecomp
    sudo ./gen/install.sh fcicomp-jpegls
    ./gen/build.sh fcicomp-H5Zjpegls release -DCMAKE_PREFIX_PATH="/usr/local/fcidecomp" -DCMAKE_INSTALL_PREFIX=/usr/local/fcidecomp
    sudo ./gen/install.sh fcicomp-H5Zjpegls

Finally, set the environment variable ``HDF5_PLUGIN_PATH`` to the install path of the compiled HDF5 plugin
specified above (following the instructions above, it is ``/usr/local/fcidecomp/hdf5/lib/plugin/``):

    export HDF5_PLUGIN_PATH=/usr/local/fcidecomp/hdf5/lib/plugin/

