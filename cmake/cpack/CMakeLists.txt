CMAKE_MINIMUM_REQUIRED (VERSION 3.4)

# Name of the project - we intentionally do not indicate any language
PROJECT (sac2c-packages NONE)

# Handle Policies
#FIXME hans: currently we set the policy to the DEFAULT
#            used by version 3.4 of CMAKE. The alternative
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
MESSAGE (STATUS "We have set our source directory to ${SAC2C_SOURCE_DIR}.")

# Include CMake config
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/sac2c-version-related.cmake")

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

# This target will cause all build dirs (including externalprojects) to be cleaned
ADD_CUSTOM_TARGET(fullclean
  COMMAND
    ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target clean
  COMMENT "Cleaning all build dirs...")

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
    BUILD_ALWAYS 1
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${_build_type}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
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
  # unset temporary variables (this prevents any overlap that may happen)
  UNSET (BINARY_DIR)
  UNSET (_build_type_name)
  UNSET (_build_type)
ENDMACRO ()

FOREACH (type ${BUILDTYPELIST})
  ADD_SAC2C_BUILD (${type})
ENDFOREACH ()

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

XINSTALL (DIRECTORY "${PROJECT_BINARY_DIR}/toplevel/symlinks/${CMAKE_INSTALL_PREFIX}/"
  DESTINATION "." USE_SOURCE_PERMISSIONS
  COMPONENT symlinks)

# now lets setup packaging (using CPack)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/cpack/config.cmake")

# vim: ts=2 sw=2 et:
