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
