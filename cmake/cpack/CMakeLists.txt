CMAKE_MINIMUM_REQUIRED (VERSION 3.4)

# Name of the project - we intentionally do not indicate any language
PROJECT (sac2c-packages NONE)

# Handle Policies
#FIXME hans: currently we set the policy to the DEFAULT
#            used by cersion 3.4 of CMAKE. The alternative
#            is to specify individual policies for different
#            features regardless of version. The danger with
#            this is that newer version will not even support
#            that policy option anymore...
CMAKE_POLICY (VERSION 3.4)

# Extra cmake packages
INCLUDE (ExternalProject)

# XXX We assume that this CMakeLists.txt file is being called from ./cmake/cpack
#     therefore, for simplicity we set the following variables, indicating the
#     actual top source directory.
GET_FILENAME_COMPONENT (SAC2C_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../" ABSOLUTE)
MESSAGE(AUTHOR_WARNING "We have set our source directory to ${SAC2C_SOURCE_DIR}.")

# Get sac2c version
# FIXME (hans): We should modularise this more - no code repetition
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/git-revision-description.cmake")
git_describe(SAC2C_VERSION --tags --abbrev=4 --dirty)
STRING (REGEX REPLACE "^v" "" SAC2C_VERSION "${SAC2C_VERSION}")
STRING (REGEX REPLACE "\n" "" SAC2C_VERSION "${SAC2C_VERSION}")
STRING (REGEX REPLACE "^([0-9]+)\\..*" "\\1" SAC2C_VERSION_MAJOR "${SAC2C_VERSION}")
STRING (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" SAC2C_VERSION_MINOR "${SAC2C_VERSION}")
SET (SAC2C_VERSION_PATCH  0)
IF ("${SAC2C_VERSION}" MATCHES "-([0-9]+)(-g[a-f0-9]+)?(-dirty)?$")
  SET (SAC2C_VERSION_PATCH  "${CMAKE_MATCH_1}")
ENDIF ()

# Let use build packages of both the debug and production release types
# debug
ExternalProject_Add(sac2c-debug
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    PREFIX sac2c-debug
    SOURCE_DIR "${SAC2C_SOURCE_DIR}"
    INSTALL_DIR "${PROJECT_BINARY_DIR}/toplevel"
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=DEBUG
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      -DPACKAGE_PREFIX=${PROJECT_BINARY_DIR}/toplevel
)
# release
ExternalProject_Add(sac2c-release
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    PREFIX sac2c-release
    SOURCE_DIR "${SAC2C_SOURCE_DIR}"
    INSTALL_DIR "${PROJECT_BINARY_DIR}/toplevel"
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=RELEASE
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      -DPACKAGE_PREFIX=${PROJECT_BINARY_DIR}/toplevel
)

# Ensure that we can create symbolic links - only possible on UNIX systems
IF (NOT UNIX)
  MESSAGE (FATAL_ERROR "We are unable to generate packages on non-Unix systems at the moment")
ENDIF ()

# create symbolic links to binaries
# XXX(hans) we should not do this for distributable packages
# INSTALL (CODE
#   "FILE (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/toplevel/${CMAKE_INSTALL_PREFIX}/bin)
#   FILE (GLOB bin_files LIST_DIRECTORIES FALSE
#     RELATIVE ${PROJECT_BINARY_DIR}/toplevel/${CMAKE_INSTALL_PREFIX}/libexec/sac2c/${SAC2C_VERSION}/
#     ${PROJECT_BINARY_DIR}/toplevel/${CMAKE_INSTALL_PREFIX}/libexec/sac2c/${SAC2C_VERSION}/*_?
#   )
#   FOREACH (bin \${bin_files})
#     EXECUTE_PROCESS (COMMAND
#       ${CMAKE_COMMAND} -E create_symlink
#         \"${CMAKE_INSTALL_PREFIX}/libexec/sac2c/${SAC2C_VERSION}/\${bin}\"
#         \"${PROJECT_BINARY_DIR}/toplevel/${CMAKE_INSTALL_PREFIX}/bin/\${bin}\"
#     )
#   ENDFOREACH ()"
#   COMPONENT symlinks
# )

# install the thing to root
INSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "${CMAKE_INSTALL_PREFIX}" USE_SOURCE_PERMISSIONS
  COMPONENT complete)

# now lets setup packaging (using CPack)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/cpack/config.cmake")

# vim: ts=2 sw=2 et:
