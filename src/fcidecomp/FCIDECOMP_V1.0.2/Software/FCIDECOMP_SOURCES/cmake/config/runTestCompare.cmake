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

# runTestCompare.cmake executes a command that generates an output
# file. The output file is then compared against a reference file. Exit
# status of command can also be checked.

# Arguments checking
IF (NOT TEST_NAME)
  MESSAGE (FATAL_ERROR "Require TEST_NAME to be defined")
ENDIF (NOT TEST_NAME)
IF (NOT TEST_PROGRAM)
  MESSAGE (FATAL_ERROR "Require TEST_PROGRAM to be defined")
ENDIF (NOT TEST_PROGRAM)
IF (NOT TEST_ARGS)
 MESSAGE (STATUS "Require TEST_ARGS to be defined")
ENDIF (NOT TEST_ARGS)
IF (NOT TEST_FOLDER)
  MESSAGE (FATAL_ERROR "Require TEST_FOLDER to be defined")
ENDIF (NOT TEST_FOLDER)

# Arguments checking for the stdout comparisons 
IF (NOT TEST_REF_OUTPUT)
  SET (TEST_SKIP_COMPARE_OUTPUT ON)
ENDIF(NOT TEST_REF_OUTPUT)

# Arguments checking for the stderr comparisons 
IF (NOT TEST_REF_ERR)
  SET (TEST_SKIP_COMPARE_ERR ON)
ENDIF(NOT TEST_REF_ERR)

# Arguments checking for the output files comparisons 
IF ( (NOT TEST_REF_OUTPUT_FILE) AND (NOT TEST_REF_OUTPUT_FILE_LIST) ) 
  SET (TEST_SKIP_COMPARE_OUTPUT_FILES ON)
ENDIF()

# Check that the ouput file is defined
IF( (NOT TEST_OUTPUT_FILE_LIST) AND (NOT TEST_OUTPUT_FILE) )
  IF (NOT TEST_SKIP_COMPARE_OUTPUT_FILES)
    MESSAGE (FATAL_ERROR "Require TEST_OUTPUT_FILE or TEST_OUTPUT_FILE_LIST to be defined")
  ENDIF (NOT TEST_SKIP_COMPARE_OUTPUT_FILES)
ENDIF ()

# Check that the ouput file list is defined
IF ( (NOT TEST_OUTPUT_FILE_LIST) AND (TEST_OUTPUT_FILE) )
  SET (TEST_OUTPUT_FILE_LIST ${TEST_OUTPUT_FILE})
ENDIF ()
# Check that the reference ouput file list is defined
IF ( (NOT TEST_REF_OUTPUT_FILE_LIST) AND (TEST_REF_OUTPUT_FILE) )
  SET (TEST_REF_OUTPUT_FILE_LIST ${TEST_REF_OUTPUT_FILE})
ENDIF ()

# Replace ";" by space in the list of arguments
STRING (REPLACE ";" " " TEST_ARGS_STRING "${TEST_ARGS}")
MESSAGE (STATUS "COMMAND: ${TEST_PROGRAM} ${TEST_ARGS_STRING}")

IF (TEST_ENV_VAR)
#  MESSAGE (STATUS "TEST_ENV_VAR: $ENV{${TEST_ENV_VAR}}")
  SET (ENV{${TEST_ENV_VAR}} "${TEST_ENV_VALUE}") 
#  MESSAGE (STATUS "TEST_ENV_VAR: $ENV{${TEST_ENV_VAR}}")
ENDIF (TEST_ENV_VAR)

# # Remove the output file if it already exists
# EXECUTE_PROCESS (
#   COMMAND ${CMAKE_COMMAND}
#   -E remove
#   ${TEST_OUTPUT_FILE}
#   )

SET (TEST_OUTPUT "${TEST_NAME}.out")
SET (TEST_ERR "${TEST_NAME}.err")

# run the test program, capture the stdout/stderr and the result var
EXECUTE_PROCESS (
  COMMAND ${TEST_PROGRAM} ${TEST_ARGS}
  WORKING_DIRECTORY ${TEST_FOLDER}
  RESULT_VARIABLE TEST_RESULT
  OUTPUT_FILE ${TEST_OUTPUT}
  ERROR_FILE ${TEST_ERR}
  )

MESSAGE (STATUS "COMMAND Result: ${TEST_RESULT}")

# if the return value is !=${TEST_EXPECT} bail out
IF (NOT ${TEST_RESULT} STREQUAL ${TEST_EXPECT})
  MESSAGE ( FATAL_ERROR "Failed: Test program ${TEST_PROGRAM} exited != ${TEST_EXPECT}.\n${TEST_ERROR}")
ENDIF (NOT ${TEST_RESULT} STREQUAL ${TEST_EXPECT})

# IF (TEST_ERROR)
#   MESSAGE (STATUS "COMMAND Error:\n${TEST_ERROR}")
# ENDIF (TEST_ERROR)

IF (NOT TEST_SKIP_COMPARE_OUTPUT)
  # now compare the output with the reference
  EXECUTE_PROCESS (
      COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FOLDER}/${TEST_OUTPUT} ${TEST_REF_OUTPUT}
      RESULT_VARIABLE TEST_RESULT
  )
  MESSAGE (STATUS "COMPARE stdout result: ${TEST_RESULT}")
  # again, if return value is !=0 scream and shout
  IF (NOT ${TEST_RESULT} STREQUAL 0)
    MESSAGE (FATAL_ERROR "Failed: The content of ${TEST_FOLDER}/${TEST_OUTPUT} did not match ${TEST_REF_OUTPUT}")
    # print the content of the files
    FILE (READ ${TEST_REF_OUTPUT} REFERENCE_OUTPUT)
    MESSAGE (STATUS "Expected output:\n${REFERENCE_OUTPUT}")
    FILE (READ ${TEST_FOLDER}/${TEST_OUTPUT} GENERATED_OUTPUT)
    MESSAGE (STATUS "Obtained output:\n${GENERATED_OUTPUT}")
  ENDIF (NOT ${TEST_RESULT} STREQUAL 0)
ENDIF (NOT TEST_SKIP_COMPARE_OUTPUT)

IF (NOT TEST_SKIP_COMPARE_ERR)
  # now compare the output with the reference
  EXECUTE_PROCESS (
      COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FOLDER}/${TEST_ERR} ${TEST_REF_ERR}
      RESULT_VARIABLE TEST_RESULT
  )
  MESSAGE (STATUS "COMPARE stderr result: ${TEST_RESULT}")
  # again, if return value is !=0 scream and shout
  IF (NOT ${TEST_RESULT} STREQUAL 0)
    MESSAGE (FATAL_ERROR "Failed: The content of ${TEST_FOLDER}/${TEST_ERR} did not match ${TEST_REF_ERR}")
    # print the content of the files
    FILE (READ ${TEST_REF_ERR} REFERENCE_ERR)
    MESSAGE (STATUS "Expected error:\n${REFERENCE_ERR}")
    FILE (READ ${TEST_FOLDER}/${TEST_ERR} GENERATED_ERR)
    MESSAGE (STATUS "Obtained error:\n${GENERATED_ERR}")
  ENDIF (NOT ${TEST_RESULT} STREQUAL 0)
ENDIF (NOT TEST_SKIP_COMPARE_ERR)

IF (NOT TEST_SKIP_COMPARE_OUTPUT_FILES)

  LIST (LENGTH TEST_OUTPUT_FILE_LIST  TEST_OUTPUT_FILE_LIST_LEN)
  MATH (EXPR NITEMS "${TEST_OUTPUT_FILE_LIST_LEN} - 1")

  FOREACH (ITEM RANGE "${NITEMS}")
    LIST (GET TEST_OUTPUT_FILE_LIST ${ITEM} TEST_OUTPUT_FILE)
    LIST (GET TEST_REF_OUTPUT_FILE_LIST ${ITEM} TEST_REF_OUTPUT_FILE)
  
    # now compare the output file with the reference file
    EXECUTE_PROCESS (
      COMMAND ${CMAKE_COMMAND}
      -E compare_files 
      ${TEST_FOLDER}/${TEST_OUTPUT_FILE} ${TEST_REF_OUTPUT_FILE}
      RESULT_VARIABLE TEST_RESULT
      )
    MESSAGE (STATUS "COMPARE output files result: ${TEST_RESULT}")
    # again, if return value is !=0 scream and shout
    IF (NOT ${TEST_RESULT} STREQUAL 0)
      MESSAGE (FATAL_ERROR "Failed: The file ${TEST_FOLDER}/${TEST_OUTPUT_FILE} did not match ${TEST_REF_OUTPUT_FILE}")
    ENDIF (NOT ${TEST_RESULT} STREQUAL 0)
  ENDFOREACH()

ENDIF (NOT TEST_SKIP_COMPARE_OUTPUT_FILES)

# # Remove the files that have been generated during the test
# EXECUTE_PROCESS (
#   COMMAND ${CMAKE_COMMAND}
#   -E remove 
#   ${TEST_OUTPUT_FILE}
#   )

# everything went fine...
MESSAGE ("Passed: The output of ${TEST_PROGRAM} matches ${TEST_REFERENCE}")
