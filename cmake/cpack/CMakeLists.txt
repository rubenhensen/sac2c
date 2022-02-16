CMAKE_MINIMUM_REQUIRED (VERSION 3.19)

# Name of the project - we intentionally do not indicate any language
PROJECT (sac2c-packages NONE)

# Extra cmake packages
INCLUDE (ExternalProject)

# CMake policy handling
IF (POLICY CMP0114)
  # ExternalProject_Add() dependency handling
  CMAKE_POLICY (SET CMP0114 OLD)
ENDIF ()

# XXX We assume that this CMakeLists.txt file is being called from ./cmake/cpack
#     therefore, for simplicity we set the following variables, indicating the
#     actual top source directory.
GET_FILENAME_COMPONENT (SAC2C_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../" ABSOLUTE)
MESSAGE (STATUS "We have set our source directory to ${SAC2C_SOURCE_DIR}.")

# Include CMake config
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/sac2c-version-related.cmake")
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/sac2c/version-manager-related.cmake")

# Get the sac2c repository version
GET_SAC2C_VERSION (SAC2C_VERSION SAC2C_VERSION_MAJOR SAC2C_VERSION_MINOR SAC2C_VERSION_PATCH)
MESSAGE (STATUS "SaC2C Version is `${SAC2C_VERSION}'")

# Set what variants of sac2c we want to build
SET (BUILDTYPELIST "DEBUG;RELEASE")
MESSAGE (STATUS "Going to build `${BUILDTYPELIST}'")

# Ensure that we can create symbolic links - only possible on UNIX systems
IF (NOT UNIX)
  MESSAGE (FATAL_ERROR "We are unable to generate packages on non-Unix systems at the moment")
ENDIF ()

# We use this variable to pass arguments to the sub-cmake calls (via ExternalProject)
IF (DEFINED BUILD_OPTIONS)
  SEPARATE_ARGUMENTS (MORE_CMAKE_OPTIONS UNIX_COMMAND "${BUILD_OPTIONS}")
  MESSAGE (STATUS "Passing `${MORE_CMAKE_OPTIONS}' to sub-CMake calls")
ENDIF ()

# This target will cause all build dirs (including externalprojects) to be cleaned
ADD_CUSTOM_TARGET(fullclean
  COMMAND
    ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target clean
  COMMENT "Cleaning all build dirs...")

# Dummy target to expose `test' target in external projects
ADD_CUSTOM_TARGET(fulltest COMMENT "Testing all build dirs...")

# This macro generates an `externalproject' for a sac2c build variant.
# Is only parameter is the build type, which is one supported by the
# CMAKE_BUILD_TYPE variable.
# This macro also creates a target to do a clean of the build directory
# of the externalproject, which is attached to the target `fullclean'.
MACRO (ADD_SAC2C_BUILD _build_type)
  STRING (TOLOWER "${_build_type}" _build_type_name)
  #  Notice that we manually do the install, this is to ensure that the
  #  components (applications, libraries, etc.) are placed into a directory
  #  structure that is trivial to understand and sort through. This is needed
  #  as within this build context we still have CPack's notions of components
  #  for different package types.
  ExternalProject_Add (sac2c-${_build_type_name}
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    PREFIX "sac2c-${_build_type_name}"
    SOURCE_DIR "${SAC2C_SOURCE_DIR}"
    INSTALL_DIR "${PROJECT_BINARY_DIR}/toplevel/"
    INSTALL_COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/applications applications-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/libraries libraries-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/rtapplications rtapplications-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/sources sources-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/config config-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/headers headers-install
      COMMAND $(MAKE) DESTDIR=${PROJECT_BINARY_DIR}/toplevel/symlinks symlinks-install
    TEST_EXCLUDE_FROM_MAIN 1
    TEST_COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}/sac2c-${_build_type_name}/src/sac2c-${_build_type_name}-build" --target check
    BUILD_ALWAYS 1
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${_build_type}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      -DFUNCTESTS=${FUNCTESTS}
      -DBUILDGENERIC=${BUILDGENERIC}
      ${MORE_CMAKE_OPTIONS})
  # Get the location where the externalproject is built
  ExternalProject_Get_Property (sac2c-${_build_type_name} BINARY_DIR)
  # Create a step to clean the build directory
  ExternalProject_Add_Step (sac2c-${_build_type_name} clean
    COMMAND
      ${CMAKE_COMMAND} --build ${BINARY_DIR} --target fullclean
    DEPENDEES configure
    COMMENT "Cleaning `sac2c-${_build_type_name}', in ${BINARY_DIR}"
    EXCLUDE_FROM_MAIN 1
    ALWAYS 1)
  # Create a target which links to the clean step
  ExternalProject_Add_StepTargets(sac2c-${_build_type_name} clean)
  # Add the target as a dependency to the `fullclean' target
  ADD_DEPENDENCIES (fullclean sac2c-${_build_type_name}-clean)
  # Create step target to expose `test' and attach it to out `fulltest' target
  ExternalProject_Add_StepTargets(sac2c-${_build_type_name} test)
  ADD_DEPENDENCIES (fulltest sac2c-${_build_type_name}-test)
  # Add targets to list, to do dependencies
  LIST (APPEND SAC_TARGETS "sac2c-${_build_type_name}")
  # unset temporary variables (this prevents any overlap that may happen)
  UNSET (BINARY_DIR)
  UNSET (_build_type_name)
  UNSET (_build_type)
ENDMACRO ()

FOREACH (type ${BUILDTYPELIST})
  ADD_SAC2C_BUILD (${type})
ENDFOREACH ()

# install the thing to root
# XXX (hans) if we specify the destination as an absolute path, e.g. /usr/local
#            CPack generators RPM and DEB get confused, which leads to badly
#            build spec files, resulting in the erronous addition of tons of
#            other files. For instance, all directories and tmp files are copied
#            which leads to conflicts with other installed packages. This is why
#            the destination must be kept ".".
# XXX (hans) The directory to be copied needs to be ended by a slash '/', otherwise
#            the last part of the path is included in the install.
XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/sources/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT sources)

XINSTALL (PROGRAMS "${PROJECT_BINARY_DIR}/scripts/sac2c-version-manager"
  DESTINATION "."
  COMPONENT sources)

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/libraries/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT libraries)

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/applications/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT applications)

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/rtapplications/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT rtapplications)

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/config/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT config)

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/headers/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT headers)

# XXX (hans) NOTE: that for DEB/RPM/distributable packages (except TGZ), any
# CMAKE_INSTALL_PREFIX value given is explicily ignored/overridden by the CPACK*
# generator - as such, symlinks generated with a non-default CMAKE_INSTALL_PREFIX
# value *will be wrong*. Given though that we are only concerned with generating
# real packages for default situtations, this it OK for the moment.
XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/symlinks/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT symlinks)

# now lets setup packaging (using CPack)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/cpack/config.cmake")

# vim: ts=2 sw=2 et:
