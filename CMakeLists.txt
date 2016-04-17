CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (sac2c C)

include ("cmake/macros.cmake")
include ("cmake/options.cmake")
include ("cmake/config.cmake")
include ("cmake/sac2c/sac2crc.cmake")
include ("cmake/settings.cmake")

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "include"
  "src/include"
  "${PROJECT_BINARY_DIR}/include"
)

# Build executables for the tools.
ADD_EXECUTABLE (cb    src/maketools/cb/cb.c)
ADD_EXECUTABLE (hzip  src/maketools/hzip/hzip.c)

# FIXME(artem) We don't really need those tools.
ADD_EXECUTABLE (csd   src/maketools/csd/csd.c)
ADD_EXECUTABLE (cse   src/maketools/cse/cse.c)
ADD_EXECUTABLE (echo  src/maketools/echo/echo.c)
ADD_EXECUTABLE (icmt  src/maketools/icmt/icmt.c)
# extra tool file...
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/tools/saccc.in" "${PROJECT_BINARY_DIR}/saccc" @ONLY)

INCLUDE_DIRECTORIES (
  "${PROJECT_BINARY_DIR}"
  "src"
)

# generate files necessary to compile sac2c
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xsacdirs.h.in" "${PROJECT_BINARY_DIR}/include/sacdirs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in" "${PROJECT_BINARY_DIR}/src/build.c")
#CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf.in"
#                "${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf")

# Build sac.h
ADD_SUBDIRECTORY (src/runtime)
# Build libsac2c
ADD_SUBDIRECTORY (src/libsac2c)

# Make sac2c-related binaryies
ADD_EXECUTABLE (sac2c src/tools/sac2c/sac2c.c)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

ADD_EXECUTABLE (sac4c src/tools/sac4c/sac4c.c)
TARGET_LINK_LIBRARIES (sac4c ${DL_LIB})

ADD_EXECUTABLE (sac2tex src/tools/sac2tex/sac2tex.c)
TARGET_LINK_LIBRARIES (sac2tex ${DL_LIB})

# FIXME(artem) Is this really a dependency of sac2c?
#ADD_DEPENDENCIES (sac2c sac_h)

# Installing the sac2c project files.
# XXX(artem) in case we are doing packaging, symbolic links should be
#            avoided.
INSTALL (TARGETS sac2c RUNTIME DESTINATION "${DLL_DIR}")
IF (CMAKE_HOST_UNIX AND SYMLINKS)
    # FIXME(artem) Create symlinks for other tools we install
    INSTALL (CODE "
        FILE (MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
        EXECUTE_PROCESS (
            COMMAND ln -s ${DLL_DIR}/sac2c sac2c
            WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
    ")
ELSE ()
    INSTALL (TARGETS sac2c RUNTIME DESTINATION bin)
    INSTALL (TARGETS sac4c RUNTIME DESTINATION bin)
    INSTALL (TARGETS sac2tex RUNTIME DESTINATION bin)
ENDIF ()

INSTALL (FILES "${SAC2CRC_BUILD_CONF}" DESTINATION "${SAC2CRC_DIR}")

# Cpack information
# INCLUDE(InstallRequiredSystemLibraries)

# By setting this on we can see where installation targets are specified via
# absolute pathes.  For portability pruroses this should be avoided.
# SET (CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION ON)

SET (CPACK_GENERATOR "DEB")

# XXX(artem) This allows one to use absolute pathes during installation,
#            but it doesn't seem to work on windows and friends.
SET (CPACK_SET_DESTDIR ON)

SET (CPACK_PACKAGE_VERSION_MAJOR "${SAC2C_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR "${SAC2C_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH "${SAC2C_VERSION_PATCH}")

# SET (CPACK_PACKAGE_DESCRIPTION_FILE ...)
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY
     "The sac2c compiler for a data-parallel array-based functional language sac")
# FIXME(artem) We need to decide on where do we put the stuff on the target system...)
#SET (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jack the Ripper")

INCLUDE(CPack)
