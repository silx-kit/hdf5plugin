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

#---------------------------------------------------------------------------
# Unit Test Settings
#---------------------------------------------------------------------------
IF (BUILD_TESTING)
  ENABLE_TESTING()

  # Setting for the tests with valgrind
  IF (MEMORY_CHECK)
    INCLUDE(CTest)
    # Find valgrind program
    FIND_PROGRAM (CTEST_MEMORYCHECK_COMMAND valgrind)
    
    IF (CTEST_MEMORYCHECK_COMMAND)
      # Valgrind has been found 
      MESSAGE ("-- Check for valgrind program: ${CTEST_MEMORYCHECK_COMMAND}")
      # Set valgrind options
      SET (CTEST_MEMORYCHECK_COMMAND_OPTIONS "-v --tool=memcheck --leak-check=full --trace-children=yes --track-fds=yes --num-callers=50 --show-reachable=yes --track-origins=yes --malloc-fill=0xff --free-fill=0xfe")
    ELSE (CTEST_MEMORYCHECK_COMMAND)
      # Valgrind has not been found 
      # Disable memory check
      SET (MEMORY_CHECK OFF)
    ENDIF (CTEST_MEMORYCHECK_COMMAND)
  ENDIF (MEMORY_CHECK)

  # Setting for the coverage testing
  IF (COVERAGE_TESTING)
    INCLUDE (CTest)
  ENDIF (COVERAGE_TESTING)

ENDIF (BUILD_TESTING)
