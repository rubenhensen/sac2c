CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (sac2c C)

include ("cmake/macros.cmake")
include ("cmake/options.cmake")
include ("cmake/config.cmake")
include ("cmake/settings.cmake")

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "include"
  "src/include"
  "${PROJECT_BINARY_DIR}/include"
)

# Macro to buikd a tool.
MACRO (BUILD_TOOL ARG)
  ADD_EXECUTABLE(${ARG} src/maketools/${ARG}/${ARG}.c)
  # Copy the tool to the lcoation
  # FIXME  Why the hell we do not store them in $SAC2CBASE/bin
  #           instead of $SAC2CBASE/src/bin?
  ADD_CUSTOM_COMMAND(
          TARGET ${ARG}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${ARG}
                  ${PROJECT_SOURCE_DIR}/src/bin
)
ENDMACRO (BUILD_TOOL)

# Build executables for the tools.
BUILD_TOOL (cb)
BUILD_TOOL (csd)
BUILD_TOOL (cse)
BUILD_TOOL (echo)
BUILD_TOOL (icmt)
BUILD_TOOL (hzip)

INCLUDE_DIRECTORIES (
  "${PROJECT_BINARY_DIR}"
  "src"
)

ADD_SUBDIRECTORY (src)

# Make sac2c binary
ADD_EXECUTABLE (sac2c src/tools/sac2c/sac2c.c)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

# FIXME(artem) Is this really a dependency of sac2c?
ADD_DEPENDENCIES (sac2c sac_h)

# Installing the sac2c project files.
# XXX(artem) in case we are doing packaging, symbolic links should be
#            avoided.
INSTALL (TARGETS sac2c RUNTIME DESTINATION "${DLL_DIR}")
IF (CMAKE_HOST_UNIX AND SYMLINKS)
    INSTALL (CODE "
        FILE (MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
        EXECUTE_PROCESS (
            COMMAND ln -s ${DLL_DIR}/sac2c sac2c
            WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
    ")
ELSE ()
    INSTALL (TARGETS sac2c RUNTIME DESTINATION bin)
ENDIF ()

INSTALL (FILES "${SAC2CRC_BUILD_CONF}" DESTINATION "${SAC2CRC_DIR}")

# Cpack information
# INCLUDE(InstallRequiredSystemLibraries)

SET (CPACK_GENERATOR "DEB")

# XXX(artem) This allows one to use absolute pathes during installation,
#            but it doesn't seem to work on windows and friends.
SET (CPACK_SET_DESTDIR ON)

# FIXME(artem) Get this from the ${SAC2C_VERSION} variable
SET (CPACK_PACKAGE_VERSION_MAJOR 1)
SET (CPACK_PACKAGE_VERSION_MINOR 2)
SET (CPACK_PACKAGE_VERSION_PATCH 0)

# SET (CPACK_PACKAGE_DESCRIPTION_FILE ...)
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY
     "The sac2c compiler for a data-parallel array-based functional language sac")
# FIXME(artem) We need to decide on where do we put the stuff on the target system...)
#SET (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jack the Ripper")

INCLUDE(CPack)
