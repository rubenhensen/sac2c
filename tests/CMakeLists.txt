CMAKE_MINIMUM_REQUIRED (VERSION 3.17)

# Unit testing is done here through a collection of
# scripts, with `run.sh` being the main execution script.
# Details on how it works, and interacts with the other
# scripts, is described in the `tests` directory. In short
# though, `run.sh` generates a Makefile from a SaC source
# file. The Makefile is what is then run, meaning that we
# can provide for difference testing scenarios, e.g. just
# compiling a binary, getting phase output, etc.
#
# We copy over the *entire* tests directory into the build
# directory. We use a target, `copy-unit-tests`, to do this.
# In this way, we are able to track changes in units tests
# in the source directory, and update these correctly.
#
# Tests are _registered_ through the `REGISTER_TEST` macro.
# Here we call the `run.sh` script on the given SaC source
# file.

SET (TESTS_SRC_DIR ${PROJECT_SOURCE_DIR}/tests)
SET (TESTS_BIN_DIR ${PROJECT_BINARY_DIR}/tests)

MACRO (REGISTER_TEST sac_file)
    GET_FILENAME_COMPONENT (test_name "${sac_file}" NAME_WE)

    MESSAGE (DEBUG "Adding test ${test_name}")
    ADD_TEST (
        NAME ${test_name}
        COMMAND
            ${TESTS_BIN_DIR}/scripts/run.sh
            "${sac_file}" "${TESTS_BIN_DIR}")
    SET_TESTS_PROPERTIES (${test_name} PROPERTIES
        REQUIRED_FILES "${TESTS_BIN_DIR}/${sac_file}")
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

SET (_copy_depends)
FOREACH (f ${sac_test_files} ${headers} ${mk})
    GET_FILENAME_COMPONENT (localdir "${f}" DIRECTORY)
    GET_FILENAME_COMPONENT (fname "${f}" NAME)
    ADD_CUSTOM_COMMAND (OUTPUT "${TESTS_BIN_DIR}/${f}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS_BIN_DIR}/${localdir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${TESTS_SRC_DIR}/${f}" "${TESTS_BIN_DIR}/${localdir}"
        DEPENDS "${TESTS_SRC_DIR}/${f}"
        COMMENT "Copying ${fname}")
    LIST (APPEND _copy_depends "${TESTS_BIN_DIR}/${f}")
ENDFOREACH ()

# This target copies over the sources for all tests. It is attached to
# the ALL target, so that we do this whenever we call `make` and `make check`.
ADD_CUSTOM_TARGET (copy-unit-tests ALL
    DEPENDS ${_copy_depends}
    COMMENT "Copying over unit tests")
ADD_DEPENDENCIES (check copy-unit-tests)

# We derive _tests_ from all SaC sources which have pre-fix 'test-'
FILE (GLOB_RECURSE tests
      RELATIVE "${TESTS_SRC_DIR}" "test-*.sac")

# We check for the "REQFEATURES|" keyword in each unit file, and
# iff all ENABLE_* features are ON, do we add the test. If no features
# are required, the test is added by default.
SET (activated_tests)
FOREACH (test ${tests})
    FILE (STRINGS ${test} featuresstr REGEX "REQFEATURES\\|.*")
    IF (featuresstr)
        STRING (REGEX MATCHALL "ENABLE_[A-Za-z]+" features ${featuresstr})
        MESSAGE (DEBUG "require ${features} features for ${test}")
        SET (_isalloptionson TRUE)
        FOREACH (feature ${features})
            IF (NOT ${feature})
                MESSAGE (DEBUG "feature ${feature} is OFF, not adding ${test}")
                SET (_isalloptionson FALSE)
                BREAK ()
            ENDIF ()
        ENDFOREACH ()
        IF (${_isalloptionson})
            LIST (APPEND activated_tests ${test})
        ENDIF ()
    ELSE ()
        # add the test as there are no strict feature requirements
        LIST (APPEND activated_tests ${test})
    ENDIF ()
ENDFOREACH ()

FOREACH (test ${activated_tests})
    REGISTER_TEST ("${test}")
ENDFOREACH ()
