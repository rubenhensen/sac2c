# configure version-manager script

# minor macro that checks to see if a python module is available or not
MACRO (CHECK_PYTHON_MODULE module)
    EXECUTE_PROCESS (COMMAND "${Python3_EXECUTABLE}" "-c"
        "import ${module}"
        RESULT_VARIABLE _module_status
        ERROR_QUIET)

    IF (_module_status) # not found
        MESSAGE (FATAL_ERROR "Python3 module `${module}' is missing!")
    ELSE ()
        MESSAGE (STATUS "Python3 module `${module}' found")
    ENDIF ()
    UNSET (_module_status)
ENDMACRO ()

# The version manager requires Python 3.
SET (Python3_FIND_UNVERSIONED_NAMES FIRST)
FIND_PACKAGE (Python3 REQUIRED)

# make sure we have the Python argparse module available
# CHECK_PYTHON_MODULE ("argparse") as of Python3 this is built-in!

# create a dictionary that relates build type and the build postfix
SET (BUILD_TYPE_POSTFIX_DICT)
FOREACH (t ${KNOWN_BUILD_TYPES})
  STRING (APPEND BUILD_TYPE_POSTFIX_DICT "'${t}': '${CMAKE_${t}_POSTFIX}',")
ENDFOREACH ()
SET (BUILD_TYPE_POSTFIX_DICT "{ ${BUILD_TYPE_POSTFIX_DICT} }")

# create a dictionary that relates build type and the name of the directory
# used during the installation process
SET (BUILD_TYPE_DIRECTORY_DICT)
FOREACH (t ${KNOWN_BUILD_TYPES})
  BUILD_TYPE_TO_DIRNAME (${t} dirname)
  STRING (APPEND BUILD_TYPE_DIRECTORY_DICT "'${t}': '${dirname}',")
ENDFOREACH ()
SET (BUILD_TYPE_DIRECTORY_DICT "{ ${BUILD_TYPE_DIRECTORY_DICT} }")

CONFIGURE_FILE (
  "${SAC2C_SOURCE_DIR}/scripts/sac2c-version-manager.in"
  "${PROJECT_BINARY_DIR}/scripts/sac2c-version-manager" @ONLY)
