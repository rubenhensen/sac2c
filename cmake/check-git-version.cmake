## CMake Script
# Created 04 Nov 2016 by Hans-Nikolai Viessmann <hv15 AT hw.ac.uk>
# 
# About: 
#  This script is used to compare the state of the `__version-git.txt',
#  which contains the version state of the GIT repo at the time the CMake
#  configure phase was last run, to the actual state of the GIT repo. The
#  GIT repo state is used to set the version number of the compiler as well
#  as to indicate whether the state is clean or dirty. Within the CMake system,
#  this information is only propagated to the build system at configure time,
#  and subsequent changes to the source tree of the repo *will not* cause the
#  version to be updated *until* the user manually calls `make rebuild_cache'
#  or calls `cmake ..'. Doing this though is inconvenient, and will be easily
#  forgotten. This script does this for the user, by checking for changes in
#  the GIT repo version and causing to to re-configure during the build phase.
#
# Tested with CMake v3.6
##
##
# Parameters
#  -D GIT_COMMAND=<GIT Binary path>       (Mandatory)
#  -D BUILD_DIR=<Path to build directory> (Mandatory)
##

IF (NOT (DEFINED BUILD_DIR AND DEFINED GIT_COMMAND))
  MESSAGE (SEND_ERROR "Did you forgot to pass the right parameters?")
  RETURN ()
ENDIF ()

## Get the current state of the GIT repo
EXECUTE_PROCESS (
  COMMAND
    ${GIT_COMMAND} describe --tags --abbrev=4 --dirty
  OUTPUT_VARIABLE NVER
)

## Read the GIT repo state from last CMake configuration
FILE (READ "__version-git.txt" OVER)

## Compare both versions
IF (NOT "${NVER}" STREQUAL "${OVER}")
  MESSAGE (STATUS "Something has changed within the compiler repository, re-configuring...")
  # By updating the file we break the dependency with the configure phase
  # which will cause CMake to re-configure the build system. However, this
  # will not occur until the next build...
  FILE (REMOVE "__version-git.txt")
  # So, we force CMake to do the re-configure NOW
  EXECUTE_PROCESS(
    COMMAND
      ${CMAKE_COMMAND} --build ${BUILD_DIR} --target rebuild_cache 
    )
ENDIF ()

# vim:ts=2:sw=2:et
