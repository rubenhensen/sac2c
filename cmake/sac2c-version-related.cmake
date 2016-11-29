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
    OUTPUT_FILE
        "${PROJECT_BINARY_DIR}/__version-repo.txt")

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
        -P "${PROJECT_SOURCE_DIR}/cmake/check-repo-version.cmake"
    BYPRODUCTS
        "${PROJECT_BINARY_DIR}/__version-repo.txt"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Checking git repo version")

#
# Callable functions/macros
# 

# Get sac2c version information
FUNCTION (GET_SAC2C_VERSION version major minor patch)
    SET (_version "")
    SET (_major "")
    SET (_minor "")
    SET (_patch 0)

    FILE (READ "${PROJECT_BINARY_DIR}/__version-repo.txt" _version)
    STRING (REGEX REPLACE "^v" "" _version "${_version}")
    STRING (REGEX REPLACE "\n" "" _version "${_version}")
    STRING (REGEX REPLACE "^([0-9]+)\\..*" "\\1" _major "${_version}")
    STRING (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" _minor "${_version}")
    IF ("${_version}" MATCHES "-([0-9]+)(-g[a-f0-9]+)?(-dirty)?$")
        SET (_patch "${CMAKE_MATCH_1}")
    ENDIF ()

    SET (${version} ${_version} PARENT_SCOPE)
    SET (${major} ${_major} PARENT_SCOPE)
    SET (${minor} ${_minor} PARENT_SCOPE)
    SET (${patch} ${_patch} PARENT_SCOPE)
ENDFUNCTION ()

# If the status of git repository is "dirty", installation targets should
# be not available.
# XXX The SAC2C_IS_DIRTY flag needs to have a numerical value, as it is
#     propagated to sacdirs.h file.
MACRO (CHECK_IF_DIRTY) # oh yeah baby!
    STRING (FIND "${SAC2C_VERSION}" "dirty" _gitdirty)
    IF (_gitdirty GREATER 0) # If 'dirty' not found, we are -1, otherwise something
                            # greater than 0.
      SET(SAC2C_IS_DIRTY 1)
      MACRO (XINSTALL)
      ENDMACRO ()
      INSTALL(CODE 
          "EXECUTE_PROCESS (
              COMMAND ${CMAKE_COMMAND} -E echo 
                  \"\n\n The current build includes local changes that are\n\"
                  \"not committed to the git repository, as indicated by\n\"
                  \"'git describe --dirty'. Therefore, it is NOT POSSIBLE\n\"
                  \"to install the current version. Either get rid of local\n\"
                  \"changes or make a commit.\n\"
           )
           MESSAGE (FATAL_ERROR \"Exiting now\")"
      )
    ELSE ()
      SET(SAC2C_IS_DIRTY 0)
      MACRO (XINSTALL)
        INSTALL (${ARGV})
      ENDMACRO ()
    ENDIF ()
ENDMACRO ()
