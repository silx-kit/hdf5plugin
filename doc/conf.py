# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

import os

# See https://docs.readthedocs.io/en/stable/builds.html#build-environment
on_rtd = os.environ.get('READTHEDOCS') == 'True'


# -- Project information -----------------------------------------------------

project = 'hdf5plugin'
copyright = u'2016-2021, Data analysis unit, European Synchrotron Radiation Facility, Grenoble'
author = 'ESRF - Data Analysis Unit'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    'nbsphinx',
]

if not on_rtd:
    extensions.append('sphinx_rtd_theme')

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'default' if on_rtd else 'sphinx_rtd_theme'
