Introduction
------------

Purpose
~~~~~~~

The document describes a design proposal for a maintainable solution allowing users to reliably decode FCI L1c products
compressed with CharLS.

Applicable Documents
~~~~~~~~~~~~~~~~~~~~

.. list-table:: Applicable documents
  :header-rows: 1
  :class: longtable
  :widths: 30 30 40

  * - #
    - Title
    - Reference

  * - [FCIDECOMP_WPD]

      .. _[FCIDECOMP_WPD]:
    - Work Package Description
    - EUM/SEP/WPD/21/1244304

Reference Documents
~~~~~~~~~~~~~~~~~~~

.. list-table:: Reference documents
  :header-rows: 1
  :class: longtable
  :widths: 30 30 40

  * - #
    - Title
    - Reference

  * - [CONDA]

      .. _[CONDA]:
    - ``conda`` installation instructions
    - https://conda.io/projects/conda/en/latest/user-guide/install/index.html

  * - [CONDA_VARIANTS]

      .. _[CONDA_VARIANTS]:
    - conda-build – Build variants
    - `https://docs.conda.io/projects/conda-build/en/latest/resources/variants.html <https://docs.conda.io/projects/conda-build/en/latest/resources/variants.html>`_

  * - [FCIDECOMP_CONDA]

      .. _[FCIDECOMP_CONDA]:
    - FCIDECOMP Conda recipe developed by Martin Raspaud (SMHI)
    - `https://github.com/mraspaud/fcidecomp-conda-recipe/ <https://github.com/mraspaud/fcidecomp-conda-recipe/>`_

  * - [FCIDECOMP_LATEST]

      .. _[FCIDECOMP_LATEST]:
    - FCIDECOMP v1.0.2 repository
    - `https://sftp.eumetsat.int/public/folder/UsCVknVOOkSyCdgpMimJNQ/User-Materials/Test-Data/MTG/MTG_FCI_L1C_Enhanced-NonN_TD-272_May2020/FCI_Decompression_Software_V1.0.2/EUMETSAT-FCIDECOMP_V1.0.2.tar.gz <https://sftp.eumetsat.int/public/folder/UsCVknVOOkSyCdgpMimJNQ/User-Materials/Test-Data/MTG/MTG_FCI_L1C_Enhanced-NonN_TD-272_May2020/FCI_Decompression_Software_V1.0.2/EUMETSAT-FCIDECOMP_V1.0.2.tar.gz>`_

  * - [FCIDECOMP_TEST_DATA]

      .. _[FCIDECOMP_TEST_DATA]:
    - FCIDECOMP v1.0.2 test data
    - `https://sftp.eumetsat.int/public/folder/UsCVknVOOkSyCdgpMimJNQ/User-Materials/Test-Data/MTG/MTG_FCI_L1C_Enhanced-NonN_TD-272_May2020/ <https://sftp.eumetsat.int/public/folder/UsCVknVOOkSyCdgpMimJNQ/User-Materials/Test-Data/MTG/MTG_FCI_L1C_Enhanced-NonN_TD-272_May2020/>`_

  * - [EUMECAST_OS_SPEC]

      .. _[EUMETCAST_OS_SPEC]:
    - EUMETCast Operating System specifications
    - `https://eumetsatspace.atlassian.net/wiki/spaces/DSEC/pages/739115041/Operating+System+Specifications <https://eumetsatspace.atlassian.net/wiki/spaces/DSEC/pages/739115041/Operating+System+Specifications>`_

  * - [HDF5FILTERS]

      .. _[HDF5FILTERS]:
    - HDFGroup ``filters``
    - `https://support.hdfgroup.org/services/filters.html <https://support.hdfgroup.org/services/filters.html>`_

  * - [HDF5PLUGIN]

      .. _[HDF5PLUGIN]:
    - ``hdf5plugin`` python package
    - `https://github.com/silx-kit/hdf5plugin <https://github.com/silx-kit/hdf5plugin>`_

  * - [HDFVIEW]

      .. _[HDFVIEW]:
    - HDFView Software
    - `https://www.hdfgroup.org/downloads/hdfview/ <https://www.hdfgroup.org/downloads/hdfview/>`_

  * - [MTG4AFRICA]

      .. _[MTG4AFRICA]:
    - EUMETSAT Data Tailor mtg4africa plugin
    - `https://gitlab.eumetsat.int/data-tailor/support-to-mtg/mtg4africa <https://gitlab.eumetsat.int/data-tailor/support-to-mtg/mtg4africa>`_

  * - [NETCDF_C]

      .. _[NETCDF_C]:
    - Unidata - NetCDF-C
    - `https://docs.unidata.ucar.edu/netcdf-c/current/ <https://docs.unidata.ucar.edu/netcdf-c/current/>`_


  * - [NETCDF_JAVA]

      .. _[NETCDF_JAVA]:
    - Unidata - NetCDF-Java
    - `https://www.unidata.ucar.edu/software/netcdf-java/ <https://www.unidata.ucar.edu/software/netcdf-java/>`_


  * - [NETCDF_JAVA_GITHUB]

      .. _[NETCDF_JAVA_GITHUB]:
    - NetCDF-C for reading (nj22Config.xml) in non-Unidata netCDF-Java based tools
    - `https://github.com/Unidata/thredds/issues/1063 <https://github.com/Unidata/thredds/issues/1063>`_

  * - [NETCDF_JAVA_TPF]

      .. _[NETCDF_JAVA_TPF]:
    - Discussion on the status of support for third-part HDF filters in netCDF-java
    - `https://github.com/Unidata/netcdf-java/discussions/922 <https://github.com/Unidata/netcdf-java/discussions/922>`_

  * - [PANOPLY]

      .. _[PANOPLY]:
    - Panoply netCDF, HDF and GRIB Data Viewer
    - `https://www.giss.nasa.gov/tools/panoply/ <https://www.giss.nasa.gov/tools/panoply/>`_

  * - [VV_PLAN]

      .. _[VV_PLAN]:
    - FCIDECOMP WP Verification and validation plan
    - documentation/verification-validation-plan
