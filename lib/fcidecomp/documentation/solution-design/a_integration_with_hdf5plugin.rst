.. _a_integration_with_hdf5plugin:

Appendix - Integration with ``hdf5plugin``
------------------------------------------

Integration with the ``hdf5plugin`` Python package (:ref:`[HDF5PLUGIN] <[HDF5PLUGIN]>`) requires communication with the
package maintainers (see :ref:`fcidecomp_stakeholders`).

Such interaction is initiated in the early stage of the solution development process. The proposed solution is to have
a small package including only the FCIDECOMP plugin support (developed and maintained by B-Open) separated from the main
``hdf5plugin`` package, and to have ``hdf5plugin`` use it as a sub-module dependency.

