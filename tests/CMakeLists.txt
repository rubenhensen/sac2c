CMAKE_MINIMUM_REQUIRED (VERSION 3.4)

MACRO (REGISTER_TEST sac_file)
    GET_FILENAME_COMPONENT (namewe "${sac_file}" NAME_WE)
    SET (test_name "test-${namewe}")

    ADD_TEST (
        NAME ${test_name}
        COMMAND ${PROJECT_BINARY_DIR}/tests/scripts/run.sh ${sac_file})
ENDMACRO ()

SET (SAC2C_EXEC "${PROJECT_BINARY_DIR}/sac2c${BUILD_TYPE_POSTFIX}")
SET (SCRIPT_LIST common.sh run.sh grep-command-output.sh check-return-status.sh)

FOREACH (script ${SCRIPT_LIST})
    CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/tests/scripts/${script}.in"
        "${PROJECT_BINARY_DIR}/tests/scripts/${script}" @ONLY)
ENDFOREACH ()
CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/tests/common.mk.in"
    "${PROJECT_BINARY_DIR}/tests/common.mk" @ONLY)

FILE (GLOB sac_test_files "${PROJECT_SOURCE_DIR}/tests/*.sac")
#FILE (GLOB scripts "${PROJECT_SOURCE_DIR}/tests/*.sh")
FILE (GLOB mk "${PROJECT_SOURCE_DIR}/tests/*.mk")

FILE (COPY ${sac_test_files} ${mk}  DESTINATION "${PROJECT_BINARY_DIR}/tests")
FILE (GLOB tests "${PROJECT_BINARY_DIR}/tests/test-*.sac")

FOREACH (test ${tests})
    REGISTER_TEST ("${test}")
ENDFOREACH ()
