.. _a_improvements:

Appendix - Opportunities for improvements
-----------------------------------------

Integration of CharLS 2.0.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Integration with ``CharLS 2.0.1`` is not a straightforward task since it requires a significant restructuring of the
FCIDECOMP source code, especially in terms of references to headers and libraries (both changed with the latest
release). Such restructuring is considered part of possible future developments of the software.

Generic integration with Unidata netCDF-JAVA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Generic integration with :ref:`Unidata netCDF-JAVA based tools <[NETCDF_JAVA]>` is not a straightforward task, and
at the moment it is not considered a priority. A :ref:`brief discussion with Unidata developers <[NETCDF_JAVA_TPF]>`
on the netCDF-java support to third-party filters resulted in communication that dynamically loaded filters will be
supported in the next release of netCDF-java via a filter service provider. This provides a possible future solution
for generic integration of the FCIDECOMP filter in netCDF-java based application.

Long-term dependencies maintenance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A possible long-term solution to grant the ability to install the FCIDECOMP software even in case the remote
repositories hosting its dependencies should become unreachable is the have EUMETSAT host a proxy assets repository.
This would be the reference repository for the FCIDECOMP software, acting as intermediate with the actual repositories
and caching the required dependencies.

