# EUMETSAT FCIDECOMP Conda recipe

## Manual build and installation instructions

The FCIDECOMP software can be manually built as a `conda` package and installed using the provided `conda` recipe.

### Pre-requisites

Build of the `conda` package and its installation require:

- `conda`, installed as described
  [here](<https://conda.io/projects/conda/en/latest/user-guide/install/index.html>)

- `conda-build`, installed as described [here](<https://docs.conda.io/projects/conda-build/en/latest/>)

- a connection to internet

Build on platform Windows require also:

- `Visual Studio Community 2022`, installed as described
  [here](<https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=17>)
  (make sure to install on default location `C:\Program Files\Microsoft Visual Studio\2022\Community`)

- install also these Visual Studio workloads:

  - Desktop development with C++ (with optional package "C++ CMake tools for Windows")
  - Universal Windows Platform development
  
#### Tested compilers

The build process has been successfully tested with the following compilers:

- `gcc`/`g++ 9.3.0` on Linux platforms
- `MSVC 19.30` on Windows platforms

### Conda package build and install on Linux

To build the `conda` package, run the following command from within the `conda` directory (replace `$BUILD_DIRECTORY`
with the path to the directory where `conda` packages will be dumped):

    conda build . --output-folder $BUILD_DIRECTORY

Once `conda` packages have been successfully built, create a new `conda` environment. Let's call it `fcidecomp`, but
any valid name would do (change the following instructions accordingly):

    conda create -n fcidecomp python=3.7

Activate the environment:

    conda activate fcidecomp

Execute:

    conda install -y -c anaconda -c conda-forge -c $BUILD_DIRECTORY fcidecomp

### Conda package build and install on Windows

From the Windows menu `Start`, select the Visual 2022 folder, then open the
proper command prompt:

 - x64 Native Tools Command Prompt (to build for platform 64bit)
 - x86 Native Tools Command Prompt (to build for platform 32bit)

Then, only in case of building for platform 32bit, type:

    set CONDA_FORCE_32BIT=1

To build the `conda` package, run the following command from within the `conda` directory (replace `%BUILD_DIRECTORY%`
with the path to the directory where `conda` packages will be dumped)::

    conda build . --output-folder %BUILD_DIRECTORY%

Once `conda` packages have been successfully built, create a new `conda` environment. Let's call it `fcidecomp`, but
any valid name would do (change the following instructions accordingly):

    conda create -n fcidecomp python=3.7

Activate the environment:

    conda activate fcidecomp

Execute:

    conda install -y -c anaconda -c conda-forge -c %BUILD_DIRECTORY% fcidecomp

