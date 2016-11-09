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

MESSAGE(AUTHOR_WARNING "SRC: ${SAC2C_SOURCE_DIR}")

# Let use build packages of both the debug and production release types
# debug
ExternalProject_Add(sac2c-debug
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    PREFIX sac2c-debug
    SOURCE_DIR "${SAC2C_SOURCE_DIR}"
    INSTALL_DIR "${PROJECT_BINARY_DIR}/toplevel"
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=DEBUG
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
      -DPACKAGE_PREFIX=${PROJECT_BINARY_DIR}/toplevel
)

# vim: ts=2 sw=2 et:
