# =============================================================
#
# Copyright 2015-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# =============================================================

# AUTHORS:
# - THALES Services

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
