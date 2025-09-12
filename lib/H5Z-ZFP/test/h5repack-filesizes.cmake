#h5repack-filesizes.cmake
cmake_policy(SET CMP0007 NEW)

# arguments checking
if (NOT FILE_ORIGINAL)
  message (FATAL_ERROR "Require FILE_ORIGINAL to be defined")
endif ()
if (NOT FILE_REPACK)
  message (FATAL_ERROR "Require FILE_REPACK to be defined")
endif ()
if (NOT RATIO_LIMIT)
  message (FATAL_ERROR "Require RATIO_LIMIT to be defined")
endif ()

file(SIZE ${FILE_ORIGINAL} ORIG_SIZE)
file(SIZE ${FILE_REPACK} NEW_SIZE)
math(EXPR RATIO "(${ORIG_SIZE} * 100) / ${NEW_SIZE}")

message (STATUS "Original file")
message (STATUS "  Name: ${FILE_ORIGINAL}")
message (STATUS "  Size: ${ORIG_SIZE}")
message (STATUS "Repack file")
message (STATUS "  Name: ${FILE_REPACK}")
message (STATUS "  Size: ${NEW_SIZE}")
message (STATUS " Ratio of the file sizes: ${RATIO}")

if (${RATIO} LESS ${RATIO_LIMIT})
  message (FATAL_ERROR "Failed: The RATIO was LESS ${RATIO_LIMIT}")
endif ()

# everything went fine...
message (STATUS "RATIO Passed")

