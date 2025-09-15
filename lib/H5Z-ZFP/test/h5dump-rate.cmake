#h5dump-rate.cmake
cmake_policy(SET CMP0007 NEW)

# arguments checking
if (NOT TEST_PROGRAM)
  message (FATAL_ERROR "Require TEST_PROGRAM to be defined")
endif ()
if (NOT TEST_FILE)
  message (FATAL_ERROR "Require TEST_FILE to be defined")
endif ()
if (NOT TEST_DSET)
  message (FATAL_ERROR "Require TEST_DSET to be defined")
endif ()
if (NOT TEST_RATE)
  message (FATAL_ERROR "Require TEST_RATE to be defined")
endif ()
if (NOT RATE_START)
  message (FATAL_ERROR "Require RATE_START to be defined")
endif ()

math (EXPR EXPECTED_RATIO "64 / ${TEST_RATE}")
# run the test program, capture the stdout/stderr and the result var
execute_process (
      COMMAND ${TEST_PROGRAM} -H -d ${TEST_DSET} -p ${TEST_FILE}
      RESULT_VARIABLE TEST_RESULT
      OUTPUT_VARIABLE TEST_OUT
      ERROR_VARIABLE TEST_ERROR
)
message (STATUS "dump: ${TEST_OUT}")

#      SIZE [0-9]* ([.0-9]*:1 COMPRESSION)
string (REGEX MATCH "SIZE [0-9]* \\(([0-9]*).[0-9]*:1 COMPRESSION\\)" ACTUAL_COMPRESS ${TEST_OUT})
set (ACTUAL_RATIO ${CMAKE_MATCH_1})
message (STATUS "Compression ratio")
message (STATUS "  File:     ${TEST_FILE}")
message (STATUS "  Dataset:  ${TEST_DSET}")
message (STATUS "  Expected: ${EXPECTED_RATIO}")
message (STATUS "  Actual:   ${ACTUAL_RATIO}")

if (NOT ${ACTUAL_RATIO} EQUAL ${EXPECTED_RATIO})
  message (FATAL_ERROR "Failed: The ACTUAL_RATIO was DIFFERENT to Expected: ${EXPECTED_RATIO}")
endif ()

# everything went fine...
message (STATUS "RATIO Passed")

