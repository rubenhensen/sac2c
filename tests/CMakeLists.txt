CMAKE_MINIMUM_REQUIRED (VERSION 3.4)

SET (TESTS_SRC_DIR ${PROJECT_SOURCE_DIR}/tests)
SET (TESTS_BIN_DIR ${PROJECT_BINARY_DIR}/tests)


MACRO (REGISTER_TEST sac_file)
    GET_FILENAME_COMPONENT (namewe "${sac_file}" NAME_WE)
    SET (test_name "test-${namewe}")

    ADD_TEST (
        NAME ${test_name}
        COMMAND 
            ${PROJECT_BINARY_DIR}/tests/scripts/run.sh 
            ${sac_file} ${TESTS_BIN_DIR})
ENDMACRO ()

SET (SAC2C_EXEC "${PROJECT_BINARY_DIR}/sac2c${BUILD_TYPE_POSTFIX}")
SET (SCRIPT_LIST common.sh run.sh grep-command-output.sh
                 check-return-status.sh isalloptionson.sh)

FOREACH (script ${SCRIPT_LIST})
    CONFIGURE_FILE(
        "${TESTS_SRC_DIR}/scripts/${script}.in"
        "${TESTS_BIN_DIR}/scripts/${script}" @ONLY)
ENDFOREACH ()

CONFIGURE_FILE (
    "${TESTS_SRC_DIR}/common.mk.in"
    "${TESTS_BIN_DIR}/common.mk" @ONLY)

FILE (GLOB_RECURSE sac_test_files 
      RELATIVE "${TESTS_SRC_DIR}" "*.sac")
FILE (GLOB_RECURSE headers 
      RELATIVE "${TESTS_SRC_DIR}" "*.h")
FILE (GLOB_RECURSE mk 
      RELATIVE "${TESTS_SRC_DIR}" "*.mk")

MESSAGE ("sac_test_files: ${sac_test_files}")

FOREACH (f ${sac_test_files} ${headers} ${mk})
    GET_FILENAME_COMPONENT (localdir "${f}" DIRECTORY)
    FILE (COPY ${TESTS_SRC_DIR}/${f} 
          DESTINATION "${TESTS_BIN_DIR}/${localdir}")
ENDFOREACH ()

FILE (GLOB_RECURSE tests 
      RELATIVE "${TESTS_SRC_DIR}" "test-*.sac")

FOREACH (test ${tests})
    REGISTER_TEST ("${test}")
ENDFOREACH ()
