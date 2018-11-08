# CMake File
#
# The purpose of this file is to add the check_repo_version
# target to the build system, with a configure_file. Doing so
# ensures that anytime we change the state of the GIT repo which
# results in a change of the compiler version, we cause the build
# system to re-configure.

# This section of the file is loaded by default. Additionally
# two functions: GET_SAC2C_VERSION and CHECK_IF_DIRTY, are provided
# that retrieve the compiler version and check if it is a dirty
# build, respectively.

# Get GIT version, which we need to retrieve the repository version
FIND_PACKAGE (Git)
IF (NOT GIT_FOUND)
    MESSAGE (FATAL_ERROR "Could not find GIT on the system!")
ENDIF ()

# We create a state file of the GIT repo and use it to initiate
# automatic re-configuration of the build system.
EXECUTE_PROCESS (
    COMMAND
        ${GIT_EXECUTABLE} describe --tags --abbrev=4 --dirty
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}
    OUTPUT_FILE
        "${PROJECT_BINARY_DIR}/__version-repo.txt")

# If the state of the repo is dirty because of non-standard flags
# or other unallowed buildsystem modifications, there is no need
# to check the state of the repository.
IF (NOT SAC2C_IS_DIRTY)
    # We determine if the repository is in a dirty state or not.
    EXECUTE_PROCESS (
        COMMAND
            ${GIT_EXECUTABLE} diff-index --quiet HEAD
        WORKING_DIRECTORY
            ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE
            SAC2C_IS_DIRTY)
ENDIF ()

# This set a hook on the configuration system
CONFIGURE_FILE (
    "${PROJECT_BINARY_DIR}/__version-repo.txt"
    "${PROJECT_BINARY_DIR}/__version-repo.txt" COPYONLY)

# XXX We add a target to the build system that *MUST* dependency
# of all other targets relating to the GIT repo source files.
ADD_CUSTOM_TARGET (check_repo_version
    COMMAND
        ${CMAKE_COMMAND}
        -D GIT_COMMAND="${GIT_EXECUTABLE}"
        -D BUILD_DIR="${PROJECT_BINARY_DIR}"
        -D SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        -P "${PROJECT_SOURCE_DIR}/cmake/check-repo-version.cmake"
    BYPRODUCTS
        "${PROJECT_BINARY_DIR}/__version-repo.txt"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Checking git repo version")

# If the status of git repository is "dirty", installation targets should
# be not available.
IF (SAC2C_IS_DIRTY)
  MACRO (XINSTALL)
  ENDMACRO ()
  INSTALL(CODE 
      "EXECUTE_PROCESS (
          COMMAND ${CMAKE_COMMAND} -E echo 
              \"\n\n The state of the current build is dirty.  This is\n\"
              \"either because of additional CMAKE_C_FLAGS passed to\n\"
              \"to cmake or because of local changes that are\n\"
              \"not committed to the git repository, as indicated by\n\"
              \"'git describe --dirty'. Therefore, it is NOT POSSIBLE\n\"
              \"to install the current version. Either get rid of local\n\"
              \"changes or make a commit.\n\"
       )
       MESSAGE (FATAL_ERROR \"Exiting now\")"
  )
ELSE ()
  MACRO (XINSTALL)
    INSTALL (${ARGV})
  ENDMACRO ()
ENDIF ()

#
# Callable functions/macros
#

# Get sac2c version information
# output for example repo version: v1.3.3-MijasCosta-164-g40956,
#  version = 1.3.3-MijasCosta-164-g40956
#  major = 1
#  minor = 3.3
#  patch = 164
FUNCTION (GET_SAC2C_VERSION version major minor patch)
    SET (_version "")
    SET (_major "")
    SET (_minor "")
    SET (_patch 0)

    FILE (READ "${PROJECT_BINARY_DIR}/__version-repo.txt" _version)
    STRING (REGEX REPLACE "^v" "" _version "${_version}")
    STRING (REGEX REPLACE "\n" "" _version "${_version}")
    STRING (REGEX REPLACE "^([0-9]+)\\..*" "\\1" _major "${_version}")
    STRING (REGEX REPLACE "^([0-9]+)\\.([0-9]+\\.[0-9]+|[0-9]+).*" "\\2" _minor "${_version}")
    IF ("${_version}" MATCHES "-([0-9]+)(-g[a-f0-9]+)?(-dirty)?$")
        SET (_patch "${CMAKE_MATCH_1}")
    ENDIF ()

    SET (${version} ${_version} PARENT_SCOPE)
    SET (${major} ${_major} PARENT_SCOPE)
    SET (${minor} ${_minor} PARENT_SCOPE)
    SET (${patch} ${_patch} PARENT_SCOPE)
ENDFUNCTION ()

