# $Id$
# =============================================================
# 
# PROJECT  : FCI_COMPRESSION 
# 
# AUTHOR   : THALES Services
# 
# Copyright 2015 EUMETSAT
# 
# =============================================================
# HISTORY :
# 
# VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
#
# END-HISTORY
# =============================================================

INCLUDE (CMakePackageConfigHelpers)

SET (PACKAGE_CONFIG_DIR "share/cmake")

CONFIGURE_PACKAGE_CONFIG_FILE (${PROJECT_NAME_LOWER}-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake"
  INSTALL_DESTINATION "${PACKAGE_CONFIG_DIR}/${PROJECT_NAME_LOWER}"
  PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR)

WRITE_BASIC_PACKAGE_VERSION_FILE (
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion 
)

INSTALL (EXPORT ${PROJECT_NAME_LOWER}-targets 
  NAMESPACE fcicomp:: 
  DESTINATION "${PACKAGE_CONFIG_DIR}/${PROJECT_NAME_LOWER}")

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake
        DESTINATION "${PACKAGE_CONFIG_DIR}/${PROJECT_NAME_LOWER}" )
