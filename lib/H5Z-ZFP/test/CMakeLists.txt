set(ZFP_LIB_VERSION ${ZFP_VERSION_MAJOR}${ZFP_VERSION_MINOR}${ZFP_VERSION_PATCH})
if (ZFP_VERSION VERSION_GREATER_EQUAL 1.0.0)
  set(ZFP_LIB_VERSION ${ZFP_LIB_VERSION}${ZFP_VERSION_TWEAK})
endif ()
set(ZFP_HAS_CFP 0)
if (ZFP_CFP_ENABLED)
  set(ZFP_HAS_CFP 1)
endif()

# HDF5 docs say H5Dwrite_chunk was introduced in 1.10.2
set(HDF5_HAS_WRITE_CHUNK 0)
if ("${HDF5_VERSION}" VERSION_GREATER_EQUAL "1.10.2")
    set(HDF5_HAS_WRITE_CHUNK 1)
endif()

#------------------------------------------------------------------------------#
# Setup HDF5_PLUGIN_PATH
#------------------------------------------------------------------------------#
get_target_property(HDF5_PLUGIN_PATH h5z_zfp_shared RUNTIME_OUTPUT_DIRECTORY)
if (_isMultiConfig)
  set(HDF5_PLUGIN_PATH "${HDF5_PLUGIN_PATH}/${HDF_CFG_NAME}")
endif ()

#------------------------------------------------------------------------------#
# Plugin and library write & read tests (No check implemented)
#------------------------------------------------------------------------------#
add_executable(test_write_plugin test_write.c)
target_compile_definitions(test_write_plugin PRIVATE H5Z_ZFP_USE_PLUGIN)
target_compile_definitions(test_write_plugin PRIVATE ZFP_LIB_VERSION=0x${ZFP_LIB_VERSION})
target_link_libraries(test_write_plugin h5z_zfp_shared)

add_executable(test_read_plugin test_read.c)
target_compile_definitions(test_read_plugin PRIVATE H5Z_ZFP_USE_PLUGIN)
target_link_libraries(test_read_plugin h5z_zfp_shared)

add_executable(test_write_lib test_write.c)
target_compile_definitions(test_write_lib PRIVATE ZFP_LIB_VERSION=0x${ZFP_LIB_VERSION})
target_compile_definitions(test_write_lib PRIVATE HDF5_HAS_WRITE_CHUNK=${HDF5_HAS_WRITE_CHUNK})
target_compile_definitions(test_write_lib PRIVATE ZFP_HAS_CFP=${ZFP_HAS_CFP})
target_link_libraries(test_write_lib h5z_zfp_static)
if (ZFP_CFP_ENABLED)
  target_link_libraries(test_write_lib zfp::cfp)
endif ()

add_executable(test_read_lib test_read.c)
target_link_libraries(test_read_lib h5z_zfp_static)

if (FORTRAN_INTERFACE)
  add_executable(test_rw_fortran test_rw_fortran.F90)
  target_link_libraries(test_rw_fortran h5z_zfp_static)
  set_target_properties (test_rw_fortran PROPERTIES LINKER_LANGUAGE Fortran)
endif ()

#------------------------------------------------------------------------------#
# Tests
#------------------------------------------------------------------------------#

if (MSVC)
  string(REPLACE ";" "\\;" ESCAPED_PATH "$ENV{PATH}")
endif ()

# Rate
set(RATE_RATE 32 16 8 4)
if (MSVC)
  set(RATE_DIFF 3e-7 0.008 2.0 45)
else ()
  set(RATE_DIFF 1e-7 0.003 0.4 33)
endif ()
list(LENGTH RATE_RATE NRATES)
math(EXPR NRATES "${NRATES} - 1")
foreach (IRATE RANGE ${NRATES})
  list(GET RATE_RATE ${IRATE} RATE)
  list(GET RATE_DIFF ${IRATE} DIFF)

  add_test(
    NAME
      test-rate-write-${RATE}
    COMMAND
      $<TARGET_FILE:test_write_plugin> zfpmode=1 rate=${RATE} ofile=test_zfp-rate-${RATE}.h5
  )
  set_property(
    TEST
      test-rate-write-${RATE}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-rate-write-${RATE}
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()

  add_test(
    NAME
      test-rate-read-${RATE}
    COMMAND
      $<TARGET_FILE:test_read_plugin> max_absdiff=${DIFF} max_reldiff=${DIFF} ifile=test_zfp-rate-${RATE}.h5
  )
  set_property(
    TEST
      test-rate-read-${RATE}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-rate-read-${RATE}
      APPEND
      PROPERTY
        ENVIRONMENT "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
  set_tests_properties (test-rate-read-${RATE} PROPERTIES DEPENDS test-rate-write-${RATE})

  add_test(
    NAME
      test-rate-cleanup-${RATE}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-rate-${RATE}.h5
  )
  set_tests_properties (test-rate-cleanup-${RATE} PROPERTIES DEPENDS test-rate-read-${RATE})

  set_tests_properties(test-rate-write-${RATE}   PROPERTIES FIXTURES_SETUP    "rate-read-${RATE};rate-cleanup-${RATE}")
  set_tests_properties(test-rate-write-${RATE}   PROPERTIES FIXTURES_REQUIRED "rate-write-${RATE}")
  set_tests_properties(test-rate-read-${RATE}    PROPERTIES FIXTURES_REQUIRED "rate-read-${RATE}")
  set_tests_properties(test-rate-cleanup-${RATE} PROPERTIES FIXTURES_REQUIRED "rate-cleanup-${RATE}")
  set_tests_properties(test-rate-cleanup-${RATE} PROPERTIES FIXTURES_CLEANUP  "rate-write-${RATE};rate-read-${RATE}")

  add_test(
    NAME
      test-lib-rate-write-${RATE}
    COMMAND
      $<TARGET_FILE:test_write_lib> zfpmode=1 rate=${RATE} ofile=test_zfp-lib-rate-${RATE}.h5
  )

  add_test(
    NAME
      test-lib-rate-read-${RATE}
    COMMAND
      $<TARGET_FILE:test_read_lib> max_absdiff=${DIFF} max_reldiff=${DIFF} ifile=test_zfp-lib-rate-${RATE}.h5
  )
  set_tests_properties (test-lib-rate-read-${RATE} PROPERTIES DEPENDS test-lib-rate-write-${RATE})

  add_test(
    NAME
      test-lib-rate-cleanup-${RATE}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-lib-rate-${RATE}.h5
  )
  set_tests_properties (test-lib-rate-cleanup-${RATE} PROPERTIES DEPENDS test-lib-rate-read-${RATE})

  set_tests_properties(test-lib-rate-write-${RATE}   PROPERTIES FIXTURES_SETUP    "lib-rate-read-${RATE};lib-rate-cleanup-${RATE}")
  set_tests_properties(test-lib-rate-write-${RATE}   PROPERTIES FIXTURES_REQUIRED "lib-rate-write-${RATE}")
  set_tests_properties(test-lib-rate-read-${RATE}    PROPERTIES FIXTURES_REQUIRED "lib-rate-read-${RATE}")
  set_tests_properties(test-lib-rate-cleanup-${RATE} PROPERTIES FIXTURES_REQUIRED "lib-rate-cleanup-${RATE}")
  set_tests_properties(test-lib-rate-cleanup-${RATE} PROPERTIES FIXTURES_CLEANUP  "lib-rate-write-${RATE};lib-rate-read-${RATE}")

  if (NOT MSVC AND FORTRAN_INTERFACE)
    add_test(
      NAME
        test-rate-write-${RATE}-f
      COMMAND
        $<TARGET_FILE:test_rw_fortran> zfpmode 1 rate ${RATE} ofile test_zfp_fortran-rate-${RATE}.h5
    )

    add_test(
      NAME
        test-rate-info1-${RATE}-f
      COMMAND
        "${CMAKE_COMMAND}"
        -D "TEST_PROGRAM=${HDF5_DUMP_EXECUTABLE}"
        -D "TEST_FILE=test_zfp_fortran-rate-${RATE}.h5"
        -D "TEST_DSET=compressed"
        -D "TEST_RATE=${RATE}"
        -D "RATE_START=64"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/h5dump-rate.cmake"
    )
    set_tests_properties (test-rate-info1-${RATE}-f PROPERTIES DEPENDS test-rate-write-${RATE}-f)

    add_test(
      NAME
        test-rate-info2-${RATE}-f
      COMMAND
        "${CMAKE_COMMAND}"
        -D "TEST_PROGRAM=${HDF5_DUMP_EXECUTABLE}"
        -D "TEST_FILE=test_zfp_fortran-rate-${RATE}.h5"
        -D "TEST_DSET=compressed-plugin"
        -D "TEST_RATE=${RATE}"
        -D "RATE_START=64"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/h5dump-rate.cmake"
    )
    set_tests_properties (test-rate-info2-${RATE}-f PROPERTIES DEPENDS test-rate-info1-${RATE}-f)

    add_test(
      NAME
        test-rate-cleanup-${RATE}-f
      COMMAND
        "${CMAKE_COMMAND}" -E remove test_zfp_fortran-rate-${RATE}.h5
    )
    set_tests_properties (test-rate-cleanup-${RATE}-f PROPERTIES DEPENDS test-rate-info2-${RATE}-f)

    set_tests_properties(test-rate-write-${RATE}-f   PROPERTIES FIXTURES_SETUP    "rate-info1-${RATE}-f;rate-info1-${RATE}-f;rate-cleanup-${RATE}-f")
    set_tests_properties(test-rate-write-${RATE}-f   PROPERTIES FIXTURES_REQUIRED "rate-write-${RATE}-f")
    set_tests_properties(test-rate-info1-${RATE}-f   PROPERTIES FIXTURES_REQUIRED "rate-info1-${RATE}-f")
    set_tests_properties(test-rate-info2-${RATE}-f   PROPERTIES FIXTURES_REQUIRED "rate-info2-${RATE}-f")
    set_tests_properties(test-rate-cleanup-${RATE}-f PROPERTIES FIXTURES_REQUIRED "rate-cleanup-${RATE}-f")
    set_tests_properties(test-rate-cleanup-${RATE}-f PROPERTIES FIXTURES_CLEANUP  "rate-write-${RATE}-f;rate-info1-${RATE}-f;rate-info1-${RATE}-f")
  endif ()
endforeach ()

# Accuracy
set(ACC_NO 1 2 3 4)
set(ACC_ACC 0.1 0.01 0.001 0.0001)
set(ACC_DIFF 0.025 0.004 0.0006 4e-5)
list(LENGTH ACC_NO NACCS)
math(EXPR NACCS "${NACCS} - 1")
foreach (IACC RANGE ${NACCS})
  list(GET ACC_NO ${IACC} NUMBER)
  list(GET ACC_ACC ${IACC} ACC)
  list(GET ACC_DIFF ${IACC} DIFF)

  add_test(
    NAME
      test-accuracy-write-${NUMBER}
    COMMAND
      $<TARGET_FILE:test_write_plugin> zfpmode=3 acc=${ACC} ofile=test_zfp-acc-${NUMBER}.h5
  )
  set_property(
    TEST
      test-accuracy-write-${NUMBER}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-accuracy-write-${NUMBER}
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()

  add_test(
    NAME
      test-accuracy-read-${NUMBER}
    COMMAND
      $<TARGET_FILE:test_read_plugin> ret=1 max_absdiff=${DIFF} ifile=test_zfp-acc-${NUMBER}.h5
  )
  set_property(
    TEST
      test-accuracy-read-${NUMBER}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-accuracy-read-${NUMBER}
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
  set_tests_properties (test-accuracy-read-${NUMBER} PROPERTIES DEPENDS test-accuracy-write-${NUMBER})

  add_test(
    NAME
      test-accuracy-cleanup-${NUMBER}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-acc-${NUMBER}.h5
  )
  set_tests_properties (test-accuracy-cleanup-${NUMBER} PROPERTIES DEPENDS test-accuracy-read-${NUMBER})

  set_tests_properties(test-accuracy-write-${NUMBER}   PROPERTIES FIXTURES_SETUP    "accuracy-read-${NUMBER};accuracy-cleanup-${NUMBER}")
  set_tests_properties(test-accuracy-write-${NUMBER}   PROPERTIES FIXTURES_REQUIRED "accuracy-write-${NUMBER}")
  set_tests_properties(test-accuracy-read-${NUMBER}    PROPERTIES FIXTURES_REQUIRED "accuracy-read-${NUMBER}")
  set_tests_properties(test-accuracy-cleanup-${NUMBER} PROPERTIES FIXTURES_REQUIRED "accuracy-cleanup-${NUMBER}")
  set_tests_properties(test-accuracy-cleanup-${NUMBER} PROPERTIES FIXTURES_CLEANUP  "accuracy-write-${NUMBER};accuracy-read-${NUMBER}")

  add_test(
    NAME
      test-lib-accuracy-write-${NUMBER}
    COMMAND
      $<TARGET_FILE:test_write_lib> zfpmode=3 acc=${ACC} ofile=test_zfp-lib-acc-${NUMBER}.h5
  )

  add_test(
    NAME
      test-lib-accuracy-read-${NUMBER}
    COMMAND
      $<TARGET_FILE:test_read_lib> ret=1 max_absdiff=${DIFF} ifile=test_zfp-lib-acc-${NUMBER}.h5
  )
  set_tests_properties (test-lib-accuracy-read-${NUMBER} PROPERTIES DEPENDS test-lib-accuracy-write-${NUMBER})

  add_test(
    NAME
      test-lib-accuracy-cleanup-${NUMBER}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-lib-acc-${NUMBER}.h5
  )
  set_tests_properties (test-lib-accuracy-cleanup-${NUMBER} PROPERTIES DEPENDS test-lib-accuracy-read-${NUMBER})

  set_tests_properties(test-lib-accuracy-write-${NUMBER}   PROPERTIES FIXTURES_SETUP    "lib-accuracy-read-${NUMBER};lib-accuracy-cleanup-${NUMBER}")
  set_tests_properties(test-lib-accuracy-write-${NUMBER}   PROPERTIES FIXTURES_REQUIRED "lib-accuracy-write-${NUMBER}")
  set_tests_properties(test-lib-accuracy-read-${NUMBER}    PROPERTIES FIXTURES_REQUIRED "lib-accuracy-read-${NUMBER}")
  set_tests_properties(test-lib-accuracy-cleanup-${NUMBER} PROPERTIES FIXTURES_REQUIRED "lib-accuracy-cleanup-${NUMBER}")
  set_tests_properties(test-lib-accuracy-cleanup-${NUMBER} PROPERTIES FIXTURES_CLEANUP  "lib-accuracy-write-${NUMBER};lib-accuracy-read-${NUMBER}")

  if (FORTRAN_INTERFACE)
    add_test(
      NAME
        test-accuracy-write-${NUMBER}-f
      COMMAND
        $<TARGET_FILE:test_rw_fortran> zfpmode 3 acc ${ACC} ofile test_zfp_fortran-acc-${NUMBER}.h5 write
    )

    add_test(
      NAME
        test-accuracy-diff1-${NUMBER}-f
      COMMAND
        ${HDF5_DIFF_EXECUTABLE} -v -d ${ACC} test_zfp_fortran-acc-${NUMBER}.h5 test_zfp_fortran-acc-${NUMBER}.h5 compressed original
    )
    set_property(
      TEST
        test-accuracy-diff1-${NUMBER}-f
      APPEND
      PROPERTY
        ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
    )
    if (MSVC)
      set_property(
        TEST
          test-accuracy-diff1-${NUMBER}-f
        APPEND
        PROPERTY
          ENVIRONMENT
            "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
      )
    endif ()
    set_tests_properties (test-accuracy-diff1-${NUMBER}-f PROPERTIES DEPENDS test-accuracy-write-${NUMBER}-f)

    add_test(
      NAME
        test-accuracy-diff2-${NUMBER}-f
      COMMAND
        ${HDF5_DIFF_EXECUTABLE} -v -d ${ACC} test_zfp_fortran-acc-${NUMBER}.h5 test_zfp_fortran-acc-${NUMBER}.h5 compressed-plugin original
    )
    set_property(
      TEST
        test-accuracy-diff2-${NUMBER}-f
      APPEND
      PROPERTY
        ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
    )
    if (MSVC)
      set_property(
        TEST
          test-accuracy-diff2-${NUMBER}-f
        APPEND
        PROPERTY
          ENVIRONMENT
            "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
      )
    endif ()
    set_tests_properties (test-accuracy-diff2-${NUMBER}-f PROPERTIES DEPENDS test-accuracy-diff1-${NUMBER}-f)

    add_test(
      NAME
        test-accuracy-cleanup-${NUMBER}-f
      COMMAND
        "${CMAKE_COMMAND}" -E remove test_zfp_fortran-acc-${NUMBER}.h5
    )
    set_tests_properties (test-accuracy-cleanup-${NUMBER}-f PROPERTIES DEPENDS test-accuracy-diff2-${NUMBER}-f)

    set_tests_properties(test-accuracy-write-${NUMBER}-f   PROPERTIES FIXTURES_SETUP    "accuracy-diff1-${NUMBER}-f;accuracy-diff2-${NUMBER}-f;accuracy-cleanup-${NUMBER}-f")
    set_tests_properties(test-accuracy-write-${NUMBER}-f   PROPERTIES FIXTURES_REQUIRED "accuracy-write-${NUMBER}-f")
    set_tests_properties(test-accuracy-diff1-${NUMBER}-f   PROPERTIES FIXTURES_REQUIRED "accuracy-diff1-${NUMBER}-f")
    set_tests_properties(test-accuracy-diff2-${NUMBER}-f   PROPERTIES FIXTURES_REQUIRED "accuracy-diff2-${NUMBER}-f")
    set_tests_properties(test-accuracy-cleanup-${NUMBER}-f PROPERTIES FIXTURES_REQUIRED "accuracy-cleanup-${NUMBER}-f")
    set_tests_properties(test-accuracy-cleanup-${NUMBER}-f PROPERTIES FIXTURES_CLEANUP  "accuracy-write-${NUMBER}-f;accuracy-diff1-${NUMBER}-f;accuracy-diff2-${NUMBER}-f")
  endif ()
endforeach ()

# Precision
set(PREC_PREC 12 16 20 24)

if (MSVC)
  set(PREC_DIFF 0.06 0.003 7e-4 5e-5)
else ()
  set(PREC_DIFF 0.02 0.0005 5e-5 1e-6)
endif ()
list(LENGTH PREC_PREC NPRECS)
math(EXPR NPRECS "${NPRECS} - 1")
foreach (IPREC RANGE ${NPRECS})
  list(GET PREC_PREC ${IPREC} PREC)
  list(GET PREC_DIFF ${IPREC} DIFF)

  add_test(
    NAME
      test-precision-write-${PREC}
    COMMAND
      $<TARGET_FILE:test_write_plugin> zfpmode=2 prec=${PREC} ofile=test_zfp-prec-${PREC}.h5
  )
  set_property(
    TEST
      test-precision-write-${PREC}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-precision-write-${PREC}
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()

  add_test(
    NAME
      test-precision-read-${PREC}
    COMMAND
      $<TARGET_FILE:test_read_plugin> ret=2 max_reldiff=${DIFF} ifile=test_zfp-prec-${PREC}.h5
  )
  set_property(
    TEST
      test-precision-read-${PREC}
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-precision-read-${PREC}
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
  set_tests_properties (test-precision-read-${PREC} PROPERTIES DEPENDS test-precision-write-${PREC})

  add_test(
    NAME
      test-precision-cleanup-${PREC}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-prec-${PREC}.h5
  )
  set_tests_properties (test-precision-cleanup-${PREC} PROPERTIES DEPENDS test-precision-read-${PREC})

  set_tests_properties(test-precision-write-${PREC}   PROPERTIES FIXTURES_SETUP    "precision-read-${PREC};precision-cleanup-${PREC}")
  set_tests_properties(test-precision-write-${PREC}   PROPERTIES FIXTURES_REQUIRED "precision-write-${PREC}")
  set_tests_properties(test-precision-read-${PREC}    PROPERTIES FIXTURES_REQUIRED "precision-read-${PREC}")
  set_tests_properties(test-precision-cleanup-${PREC} PROPERTIES FIXTURES_REQUIRED "precision-cleanup-${PREC}")
  set_tests_properties(test-precision-cleanup-${PREC} PROPERTIES FIXTURES_CLEANUP  "precision-write-${PREC};precision-read-${PREC}")

  add_test(
    NAME
      test-lib-precision-write-${PREC}
    COMMAND
      $<TARGET_FILE:test_write_lib> zfpmode=2 prec=${PREC} ofile=test_zfp-lib-prec-${PREC}.h5
  )

  add_test(
    NAME
      test-lib-precision-read-${PREC}
    COMMAND
      $<TARGET_FILE:test_read_lib> ret=2 max_reldiff=${DIFF} ifile=test_zfp-lib-prec-${PREC}.h5
  )
  set_tests_properties (test-lib-precision-read-${PREC} PROPERTIES DEPENDS test-lib-precision-write-${PREC})

  add_test(
    NAME
      test-lib-precision-cleanup-${PREC}
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp-lib-prec-${PREC}.h5
  )
  set_tests_properties (test-lib-precision-cleanup-${PREC} PROPERTIES DEPENDS test-lib-precision-read-${PREC})

  set_tests_properties(test-lib-precision-write-${PREC}   PROPERTIES FIXTURES_SETUP    "lib-precision-read-${PREC};lib-precision-cleanup-${PREC}")
  set_tests_properties(test-lib-precision-write-${PREC}   PROPERTIES FIXTURES_REQUIRED "lib-precision-write-${PREC}")
  set_tests_properties(test-lib-precision-read-${PREC}    PROPERTIES FIXTURES_REQUIRED "lib-precision-read-${PREC}")
  set_tests_properties(test-lib-precision-cleanup-${PREC} PROPERTIES FIXTURES_REQUIRED "lib-precision-cleanup-${PREC}")
  set_tests_properties(test-lib-precision-cleanup-${PREC} PROPERTIES FIXTURES_CLEANUP  "lib-precision-write-${PREC};lib-precision-read-${PREC}")

  if (FORTRAN_INTERFACE)
    add_test(
      NAME
        test-precision-write-${PREC}-f
      COMMAND
        $<TARGET_FILE:test_rw_fortran> zfpmode 2 prec ${PREC} ofile test_zfp_fortran-prec-${PREC}.h5 write
    )

    # FIXME: Currently, replaced the difference tests because the relative difference between the two datasets is tricky in this case.
    add_test(
      NAME
        test-precision-diff1-${PREC}-f
      COMMAND
        echo ${HDF5_DIFF_EXECUTABLE} -v -p 0.00001 test_zfp_fortran-prec-${PREC}.h5 test_zfp_fortran-prec-${PREC}.h5 compressed original
    )
    set_property(
      TEST
        test-precision-diff1-${PREC}-f
      APPEND
      PROPERTY
        ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
    )
    if (MSVC)
      set_property(
        TEST
          test-precision-diff1-${PREC}-f
        APPEND
        PROPERTY
          ENVIRONMENT
            "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
      )
    endif ()
    set_tests_properties (test-precision-diff1-${PREC}-f PROPERTIES DEPENDS test-precision-write-${PREC}-f)

    add_test(
      NAME
        test-precision-diff2-${PREC}-f
      COMMAND
        echo ${HDF5_DIFF_EXECUTABLE} -v -p 0.00001 test_zfp_fortran-prec-${PREC}.h5 test_zfp_fortran-prec-${PREC}.h5 compressed-plugin original
    )
    set_property(
      TEST
        test-precision-diff2-${PREC}-f
      APPEND
      PROPERTY
        ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
    )
    if (MSVC)
      set_property(
        TEST
          test-precision-diff2-${PREC}-f
        APPEND
        PROPERTY
          ENVIRONMENT
            "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
      )
    endif ()
    set_tests_properties (test-precision-diff2-${PREC}-f PROPERTIES DEPENDS test-precision-diff1-${PREC}-f)

    add_test(
      NAME
        test-precision-cleanup-${PREC}-f
      COMMAND
        "${CMAKE_COMMAND}" -E remove test_zfp_fortran-prec-${PREC}.h5
    )
    set_tests_properties (test-precision-cleanup-${PREC}-f PROPERTIES DEPENDS test-precision-diff2-${PREC}-f)

    set_tests_properties(test-precision-write-${PREC}-f   PROPERTIES FIXTURES_SETUP    "precision-diff1-${PREC}-f;precision-diff2-${PREC}-f;precision-cleanup-${PREC}-f")
    set_tests_properties(test-precision-write-${PREC}-f   PROPERTIES FIXTURES_REQUIRED "precision-write-${PREC}-f")
    set_tests_properties(test-precision-diff1-${PREC}-f   PROPERTIES FIXTURES_REQUIRED "precision-diff1-${PREC}-f")
    set_tests_properties(test-precision-diff2-${PREC}-f   PROPERTIES FIXTURES_REQUIRED "precision-diff2-${PREC}-f")
    set_tests_properties(test-precision-cleanup-${PREC}-f PROPERTIES FIXTURES_REQUIRED "precision-cleanup-${PREC}-f")
    set_tests_properties(test-precision-cleanup-${PREC}-f PROPERTIES FIXTURES_CLEANUP  "precision-write-${PREC}-f;precision-diff1-${PREC}-f;precision-diff2-${PREC}-f")
  endif ()
endforeach ()

# Reversible
add_test(
  NAME
    test-reversible
  COMMAND
    $<TARGET_FILE:test_write_plugin> zfpmode=5 ofile=test_zfp-reversible.h5
)
set_property(
  TEST
    test-reversible
  APPEND
  PROPERTY
    ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
)
if (MSVC)
  set_property(
    TEST
      test-reversible
    APPEND
    PROPERTY
      ENVIRONMENT
        "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
  )
endif ()

add_test(
  NAME
    test-reversible-diff
  COMMAND
    $<TARGET_FILE:test_read_plugin> ret=1 max_absdiff=0 ifile=test_zfp-reversible.h5
)
set_property(
  TEST
    test-reversible-diff
  APPEND
  PROPERTY
    ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
)
if (MSVC)
  set_property(
    TEST
      test-reversible-diff
    APPEND
    PROPERTY
      ENVIRONMENT
        "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
  )
endif ()
set_tests_properties (test-reversible-diff PROPERTIES DEPENDS test-reversible)

add_test(
  NAME
    test-reversible-cleanup
  COMMAND
    "${CMAKE_COMMAND}" -E remove test_zfp-reversible.h5
)
set_tests_properties (test-reversible-cleanup PROPERTIES DEPENDS test-reversible-diff)

set_tests_properties(test-reversible         PROPERTIES FIXTURES_SETUP    "reversible-diff;reversible-cleanup")
set_tests_properties(test-reversible         PROPERTIES FIXTURES_REQUIRED "reversible")
set_tests_properties(test-reversible-diff    PROPERTIES FIXTURES_REQUIRED "reversible-diff")
set_tests_properties(test-reversible-cleanup PROPERTIES FIXTURES_REQUIRED "reversible-cleanup")
set_tests_properties(test-reversible-cleanup PROPERTIES FIXTURES_CLEANUP  "reversible;reversible-diff")

add_test(
  NAME
    test-lib-reversible
  COMMAND
    $<TARGET_FILE:test_write_lib> zfpmode=5 ofile=test_zfp-lib-reversible.h5
)

add_test(
  NAME
    test-lib-reversible-diff
  COMMAND
    $<TARGET_FILE:test_read_lib> ret=1 max_absdiff=0 ifile=test_zfp-lib-reversible.h5
)
set_tests_properties (test-lib-reversible-diff PROPERTIES DEPENDS test-lib-reversible)

add_test(
  NAME
    test-lib-reversible-cleanup
  COMMAND
    "${CMAKE_COMMAND}" -E remove test_zfp-lib-reversible.h5
)
set_tests_properties (test-lib-reversible-cleanup PROPERTIES DEPENDS test-lib-reversible-diff)

set_tests_properties(test-lib-reversible         PROPERTIES FIXTURES_SETUP    "lib-reversible-diff;lib-reversible-cleanup")
set_tests_properties(test-lib-reversible         PROPERTIES FIXTURES_REQUIRED "lib-reversible")
set_tests_properties(test-lib-reversible-diff    PROPERTIES FIXTURES_REQUIRED "lib-reversible-diff")
set_tests_properties(test-lib-reversible-cleanup PROPERTIES FIXTURES_REQUIRED "lib-reversible-cleanup")
set_tests_properties(test-lib-reversible-cleanup PROPERTIES FIXTURES_CLEANUP  "lib-reversible;lib-reversible-diff")

if (FORTRAN_INTERFACE)
  add_test(
    NAME
      test-reversible-write-f
    COMMAND
      $<TARGET_FILE:test_rw_fortran> zfpmode 5 ofile test_zfp_fortran-reversible.h5 write
  )

  add_test(
    NAME
      test-reversible-diff1-f
    COMMAND
      ${HDF5_DIFF_EXECUTABLE} -v -p 0.00001 test_zfp_fortran-reversible.h5 test_zfp_fortran-reversible.h5 compressed original
  )
  set_property(
    TEST
      test-reversible-diff1-f
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-reversible-diff1-f
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
    set_tests_properties (test-reversible-diff1-f PROPERTIES DEPENDS test-reversible-write-f)

  add_test(
    NAME
      test-reversible-diff2-f
    COMMAND
      ${HDF5_DIFF_EXECUTABLE} -v -p 0.00001 test_zfp_fortran-reversible.h5 test_zfp_fortran-reversible.h5 compressed-plugin original
  )
  set_property(
    TEST
      test-reversible-diff2-f
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-reversible-diff2-f
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
  set_tests_properties (test-reversible-diff2-f PROPERTIES DEPENDS test-reversible-diff1-f)

  add_test(
    NAME
      test-reversible-cleanup-f
    COMMAND
      "${CMAKE_COMMAND}" -E remove test_zfp_fortran-reversible.h5
  )
  set_tests_properties (test-reversible-cleanup-f PROPERTIES DEPENDS test-reversible-diff2-f)

  set_tests_properties(test-reversible-write-f   PROPERTIES FIXTURES_SETUP    "reversible-diff1-f;reversible-diff2-f;reversible-cleanup-f")
  set_tests_properties(test-reversible-write-f   PROPERTIES FIXTURES_REQUIRED "reversible-write-f")
  set_tests_properties(test-reversible-diff1-f   PROPERTIES FIXTURES_REQUIRED "reversible-diff1-f")
  set_tests_properties(test-reversible-diff2-f   PROPERTIES FIXTURES_REQUIRED "reversible-diff2-f")
  set_tests_properties(test-reversible-cleanup-f PROPERTIES FIXTURES_REQUIRED "reversible-cleanup-f")
  set_tests_properties(test-reversible-cleanup-f PROPERTIES FIXTURES_CLEANUP  "reversible-write-f;reversible-diff1-f;reversible-diff2-f")
endif ()

#------------------------------------------------------------------------------#
# Misc tests
#------------------------------------------------------------------------------#
# Integer
add_test(NAME test-lib-int-write   COMMAND $<TARGET_FILE:test_write_lib> zfpmode=3 doint=1 ofile=test_zfp-int.h5)
add_test(NAME test-lib-int-read    COMMAND $<TARGET_FILE:test_read_lib> ret=1 max_absdiff=2 ifile=test_zfp-int.h5)
set_tests_properties (test-lib-int-read PROPERTIES DEPENDS test-lib-int-write)
add_test(NAME test-lib-int-cleanup COMMAND "${CMAKE_COMMAND}" -E remove test_zfp-int.h5)
set_tests_properties (test-lib-int-cleanup PROPERTIES DEPENDS test-lib-int-read)

set_tests_properties(test-lib-int-write   PROPERTIES FIXTURES_SETUP    "lib-int-read;lib-int-cleanup")
set_tests_properties(test-lib-int-write   PROPERTIES FIXTURES_REQUIRED "lib-int-write")
set_tests_properties(test-lib-int-read    PROPERTIES FIXTURES_REQUIRED "lib-int-read")
set_tests_properties(test-lib-int-cleanup PROPERTIES FIXTURES_REQUIRED "lib-int-cleanup")
set_tests_properties(test-lib-int-cleanup PROPERTIES FIXTURES_CLEANUP  "lib-int-write;lib-int-read")

# Highd
add_test(NAME test-lib-highd         COMMAND $<TARGET_FILE:test_write_lib> highd=1 ofile=test_zfp-highd.h5)
add_test(NAME test-lib-highd-cleanup COMMAND "${CMAKE_COMMAND}" -E remove test_zfp-highd.h5)
set_tests_properties (test-lib-highd-cleanup PROPERTIES DEPENDS test-lib-highd)

set_tests_properties(test-lib-highd         PROPERTIES FIXTURES_SETUP    "lib-highd-cleanup")
set_tests_properties(test-lib-highd         PROPERTIES FIXTURES_REQUIRED "lib-highd")
set_tests_properties(test-lib-highd-cleanup PROPERTIES FIXTURES_REQUIRED "lib-highd-cleanup")
set_tests_properties(test-lib-highd-cleanup PROPERTIES FIXTURES_CLEANUP  "lib-highd")

# Sixd
add_test(NAME test-lib-sixd         COMMAND $<TARGET_FILE:test_write_lib> sixd=1 ofile=test_zfp-sixd.h5)
add_test(NAME test-lib-sixd-cleanup COMMAND "${CMAKE_COMMAND}" -E remove test_zfp-sixd.h5)
set_tests_properties (test-lib-sixd-cleanup PROPERTIES DEPENDS test-lib-sixd)

set_tests_properties(test-lib-sixd         PROPERTIES FIXTURES_SETUP    "lib-sixd-cleanup")
set_tests_properties(test-lib-sixd         PROPERTIES FIXTURES_REQUIRED "lib-sixd")
set_tests_properties(test-lib-sixd-cleanup PROPERTIES FIXTURES_REQUIRED "lib-sixd-cleanup")
set_tests_properties(test-lib-sixd-cleanup PROPERTIES FIXTURES_CLEANUP  "lib-sixd")

if (ZFP_CFP_ENABLED)
  # zfparr
  add_test(
    NAME
      test-zfparr
    COMMAND
      $<TARGET_FILE:test_write_lib> zfparr=1 rate=10
  )

  add_test(
    NAME
      test-zfparr-diff
    COMMAND
      ${HDF5_DIFF_EXECUTABLE} -v -d 0.01 test_zfp.h5 test_zfp.h5 zfparr_original zfparr_direct
  )
  set_property(
    TEST
      test-zfparr-diff
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-zfparr-diff
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()
  set_tests_properties (test-zfparr-diff PROPERTIES DEPENDS test-zfparr)

  set_tests_properties(test-zfparr      PROPERTIES FIXTURES_SETUP    "zfparr-diff")
  set_tests_properties(test-zfparr-diff PROPERTIES FIXTURES_REQUIRED "zfparr-diff")
endif ()

# Endian
# FIXME: fails due to datatype difference -> return code 2
#add_test(
#  NAME
#    test-endian
#  COMMAND
#    ${HDF5_DIFF_EXECUTABLE} -v -d 0.00001 ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_le.h5 ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_be.h5 compressed compressed
#)
#set_property(
#  TEST
#    test-endian
#  APPEND
#  PROPERTY
#    ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
#)
#if (MSVC)
#  set_property(
#    TEST
#      test-endian
#    APPEND
#    PROPERTY
#      ENVIRONMENT
#        "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
#  )
#endif ()

#------------------------------------------------------------------------------#
# Errors tests
#------------------------------------------------------------------------------#
add_executable(test_error test_error.c)
target_compile_definitions(test_write_lib PRIVATE ZFP_LIB_VERSION=0x${ZFP_LIB_VERSION})
target_link_libraries(test_error h5z_zfp_static)

add_test(NAME test-error         COMMAND $<TARGET_FILE:test_error>)
add_test(NAME test-error-cleanup COMMAND "${CMAKE_COMMAND}" -E remove test_zfp_errors.h5)
set_tests_properties (test-error-cleanup PROPERTIES DEPENDS test-error)

set_tests_properties(test-error         PROPERTIES FIXTURES_SETUP    "error-cleanup")
set_tests_properties(test-error         PROPERTIES FIXTURES_REQUIRED "error")
set_tests_properties(test-error-cleanup PROPERTIES FIXTURES_REQUIRED "error-cleanup")
set_tests_properties(test-error-cleanup PROPERTIES FIXTURES_CLEANUP  "error")

#------------------------------------------------------------------------------#
# H5repack tests
#------------------------------------------------------------------------------#
add_executable(print_h5repack_farg print_h5repack_farg.c)
target_link_libraries(print_h5repack_farg h5z_zfp_shared)

if (NOT MSVC)
  add_test(
    NAME
      test-h5repack
    COMMAND
      ${HDF5_REPACK_EXECUTABLE} -f UD=32013,0,4,3,0,3539053052,1062232653
                                -l X,Y,Z,Indexes:CHUNK=217
                                -l Indexes2:CHUNK=1517
                                -l Pressure,Pressure2,Pressure3:CHUNK=10x20x5
                                -l Pressure_2D:CHUNK=10x20
                                -l Stress,Velocity,Stress2,Velocity2,Stress3,Velocity3,VelocityZ,VelocityZ2,VelocityZ3:CHUNK=11x21x1x1
                                -l VelocityX_2D:CHUNK=21x31
                                -l XY:CHUNK=651x1
                                -l XYZ:CHUNK=217x1
                                -l XYZ2:CHUNK=1617x1
                                -l XYZ3:CHUNK=33x1
                                ${CMAKE_CURRENT_SOURCE_DIR}/mesh.h5 mesh_repack.h5
  )
  set_property(
    TEST
      test-h5repack
    APPEND
    PROPERTY
      ENVIRONMENT "HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
  )
  if (MSVC)
    set_property(
      TEST
        test-h5repack
      APPEND
      PROPERTY
        ENVIRONMENT
          "PATH=${ESCAPED_PATH}\\;${HDF5_PLUGIN_PATH}"
    )
  endif ()

  add_test(
    NAME
      test-h5repack-filesizes
    COMMAND
      "${CMAKE_COMMAND}"
      -D "FILE_ORIGINAL=${CMAKE_CURRENT_SOURCE_DIR}/mesh.h5"
      -D "FILE_REPACK=mesh_repack.h5"
      -D "RATIO_LIMIT=200"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/h5repack-filesizes.cmake"
  )
  set_tests_properties (test-h5repack-filesizes PROPERTIES DEPENDS test-h5repack)

  add_test(
    NAME
      test-h5repack-cleanup
    COMMAND
      "${CMAKE_COMMAND}" -E rm mesh_repack.h5
  )
  set_tests_properties (test-h5repack-cleanup PROPERTIES DEPENDS test-h5repack-filesizes)

  set_tests_properties(test-h5repack           PROPERTIES FIXTURES_SETUP    "h5repack-filesizes;h5repack-cleanup")
  set_tests_properties(test-h5repack           PROPERTIES FIXTURES_REQUIRED "h5repack")
  set_tests_properties(test-h5repack-filesizes PROPERTIES FIXTURES_REQUIRED "h5repack-filesizes")
  set_tests_properties(test-h5repack-cleanup   PROPERTIES FIXTURES_REQUIRED "h5repack-cleanup")
  set_tests_properties(test-h5repack-cleanup   PROPERTIES FIXTURES_CLEANUP  "h5repack;h5repack-filesizes")
endif ()

#------------------------------------------------------------------------------#
# Version compatibility tests
#------------------------------------------------------------------------------#
set(H5FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_030040.h5 ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_030235.h5 ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_110050.h5 ${CMAKE_CURRENT_SOURCE_DIR}/test_zfp_110xxx.h5)
set(RETSTAT FALSE FALSE FALSE TRUE)
list(LENGTH H5FILES NH5FILES)
math(EXPR NH5FILES "${NH5FILES} - 1")
foreach (IFILE RANGE ${NH5FILES})
  list(GET H5FILES ${IFILE} H5FILE)
  list(GET RETSTAT ${IFILE} STATUS)

  math(EXPR VERSION_NO "${IFILE} + 1")
  add_test(NAME test-version-${VERSION_NO} COMMAND $<TARGET_FILE:test_read_lib> ifile=${H5FILE} max_reldiff=0.025 ret=2)
  set_tests_properties(test-version-${VERSION_NO} PROPERTIES WILL_FAIL ${STATUS})
endforeach ()

if (FORTRAN_INTERFACE)
  set(H5FILE test_zfp_fortran_version.h5)
  math(EXPR VERSION_NO "${NH5FILES} + 2")
  add_test(NAME test-version-${VERSION_NO}-input   COMMAND $<TARGET_FILE:test_rw_fortran> write dim 1024 zfpmode 1 rate 16 ofile ${H5FILE})
  add_test(NAME test-version-${VERSION_NO}         COMMAND $<TARGET_FILE:test_read_lib> ifile=${H5FILE} max_reldiff=0.025 ret=2)
  add_test(NAME test-version-${VERSION_NO}-cleanup COMMAND "${CMAKE_COMMAND}" -E rm ${H5FILE})
  set_tests_properties(test-version-${VERSION_NO}-input   PROPERTIES FIXTURES_SETUP    "version;version-cleanup")
  set_tests_properties(test-version-${VERSION_NO}-input   PROPERTIES FIXTURES_REQUIRED "version-input")
  set_tests_properties(test-version-${VERSION_NO}         PROPERTIES FIXTURES_REQUIRED "version")
  set_tests_properties(test-version-${VERSION_NO}-cleanup PROPERTIES FIXTURES_REQUIRED "version-cleanup")
  set_tests_properties(test-version-${VERSION_NO}-cleanup PROPERTIES FIXTURES_CLEANUP  "version-input;version")
endif ()
