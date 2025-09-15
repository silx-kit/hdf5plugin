# EUMETSAT FCIDECOMP example notebooks

The available Jupyter notebooks provide examples on the different ways in which the FCIDECOMP software can be used.

## Prerequisites

- a `conda` environment with the following libraries installed:

    - the FCIDECOMP software, installed as described [here](
      <https://gitlab.eumetsat.int/sepdssme/fcidecomp/fcidecomp/-/blob/development/INSTALL.md>)
    - the `epct_plugin_mtg` library, installed as described [here]
      (<https://gitlab.eumetsat.int/data-tailor/epct_plugin_mtg/-/blob/development/README.md>)
    - the `notebook` library, installed as described [here](<https://anaconda.org/anaconda/notebook>)
    - the `xarray` library, installed as described [here](<https://anaconda.org/anaconda/xarray>)
    - the `matplotlib` library, installed as described [here](<https://anaconda.org/conda-forge/matplotlib>)

- Test data hosted [here](<https://gitlab.eumetsat.int/data-tailor/epct-test-data/-/tree/development/MTG/MTGFCIL1>).
  These data should be placed in a directories tree structured as follows (replace $EPCT_TEST_DATA_DIR
  with any chosen name):

  ```BASH
  |_$EPCT_TEST_DATA_DIR
    |_MTG
      |_MTGFCIL1
        |_<test_file_1>
        |_<test_file_2>
        |_ ...
  ```

  Once this is done, the environment variable `EPCT_TEST_DATA_DIR` should be set to the full path to $EPCT_TEST_DATA_DIR


## Setting up the environment

In order to make the `conda` environment available from within the Jupyter notebooks, the following lines of code need
to be executed. First, activate the `conda` environment. Let's call it `fcidecomp`, but any valid name would do
(change the following instructions accordingly):

    conda activate fcidecomp


Install `ipykernel`:

    conda install -c anaconda ipykernel


Enable the `conda` environment in Jupyter `notebook`:

    python -m ipykernel install --user --name=fcidecomp


## Run the notebooks

To start Jupyter `notebook`, run:

    jupyter notebook

After selecting the notebook, select the `fcidecomp` environment from the toolbar menu `Kernel` > `Change kernel`.


