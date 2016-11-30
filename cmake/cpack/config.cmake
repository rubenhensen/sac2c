IF (NOT DEFINED SAC2C_VERSION OR "${SAC2C_VERSION}" STREQUAL "")
  MESSAGE (FATAL_ERROR "Variable SAC2C_VERSION has not been set!")
ENDIF ()

IF (NOT DEFINED SAC2C_IS_DIRTY OR "${SAC2C_IS_DIRTY}" STREQUAL "")
  MESSAGE (FATAL_ERROR "Variable SAC2C_IS_DIRTY has not been set!")
ENDIF ()

# FIXME (hans): this is not very clean...
IF (NOT DEFINED SAC2C_SOURCE_DIR OR "${SAC2C_SOURCE_DIR}" STREQUAL "")
  MESSAGE (FATAL_ERROR "Variable SAC2C_SOURCE_DIR has not been set!"
    " One solution would be to make SAC2C_SOURCE_DIR == PROJECT_SOURCE_DIR.")
ENDIF ()

# By setting this on we can see where installation targets are specified via
# absolute paths. XXX (???) For portability purposes this should be avoided.
SET (CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION ON)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET (CPACK_GENERATOR "Bundle;TGZ")
ELSE ()
  SET (CPACK_GENERATOR "RPM;DEB;TGZ")
ENDIF ()

SET (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
SET (CPACK_RPM_COMPONENT_INSTALL ON)
SET (CPACK_DEB_COMPONENT_INSTALL ON)
SET (CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

# We create separate config files for different generators
set(CPACK_PROJECT_CONFIG_FILE "${SAC2C_SOURCE_DIR}/cmake/cpack/options.cmake")

# We need to make sure that we name the output package sanely.
IF ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  # we assume that an empty build type means we are building both.
  SET (PACKAGE_POSTFIX "omnibus")
ELSE ()
  STRING (TOLOWER "${CMAKE_BUILD_TYPE}" PACKAGE_POSTFIX)
ENDIF ()

# Set default CPack Packaging options
SET (CPACK_PACKAGE_NAME              "sac2c-compiler")
SET (CPACK_PACKAGE_VENDOR            "SaC Development Team")
SET (CPACK_PACKAGE_CONTACT           "info@sac-home.org")
SET (CPACK_PACKAGE_VERSION           "${SAC2C_VERSION}")
SET (CPACK_PACKAGE_VERSION_MAJOR     "${SAC2C_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR     "${SAC2C_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH     "${SAC2C_VERSION_PATCH}")
SET (CPACK_PACKAGE_FILE_NAME         "sac2c-${SAC2C_VERSION}-${PACKAGE_POSTFIX}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "sac2c-${SAC2C_VERSION}") # XXX is this really needed?
SET (CPACK_PACKAGE_ICON              "${SAC2C_SOURCE_DIR}/cmake/cpack/dmg-bundle/sac_logo.png")

# SET (CPACK_PACKAGE_DESCRIPTION_FILE ...)
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "The sac2c compiler for a data-parallel array-based functional language SAC")
# FIXME(artem) We need to decide on where do we put the stuff on the target system...)
#SET (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET (CPACK_RESOURCE_FILE_LICENSE "${SAC2C_SOURCE_DIR}/LICENSE")

#SET (CPACK_COMPONENTS_ALL applications libraries headers conf)
# Set the displayed names for each of the components to install.
# These will be displayed in the list of components inside the installer.
SET (CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "SaC Binaries")
SET (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "SaC Libraries")
SET (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "SaC Headers")
SET (CPACK_COMPONENT_CONF_DISPLAY_NAME "SaC Configs")

# DMG Bundle related configuration
SET (CPACK_BUNDLE_NAME "sac2c-${SAC2C_VERSION}-omnibus")
## Use Macports 'makeicns' tool to generate MAC OS X icons
SET (CPACK_BUNDLE_ICON "${SAC2C_SOURCE_DIR}/cmake/cpack/dmg-bundle/sac.icns")
SET (CPACK_BUNDLE_PLIST "${PROJECT_BINARY_DIR}/Info.plist")
SET (CPACK_BUNDLE_STARTUP_COMMAND "${PROJECT_BINARY_DIR}/Install.command")
CONFIGURE_FILE ("${SAC2C_SOURCE_DIR}/cmake/cpack/dmg-bundle/Info.plist.in" "${PROJECT_BINARY_DIR}/Info.plist")
CONFIGURE_FILE ("${SAC2C_SOURCE_DIR}/cmake/cpack/dmg-bundle/Install.command.in" "${PROJECT_BINARY_DIR}/Install.command")

# Debian-specific variables
SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
#SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # non-functional
# FIXME Can we auto-generate these dependencies?
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "gcc, libc6 (>= 2.13), uuid-runtime (>= 2.20)")

# RPM-specific variables
# XXX (hans): this may not be exhaustive - does not take into account if the user
# changes the install prefix
SET (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/local /usr/local/bin /usr/local/include /usr/local/lib /usr/local/libexec /usr/local/share)
# FIXME Can we auto-generate these dependencies?
SET (CPACK_RPM_PACKAGE_REQUIRES "gcc") # we don't need to go crazy here as rpmbuild handles most of this for us

# Disable CPack if we are dirty
IF (NOT SAC2C_IS_DIRTY)
  INCLUDE (CPack)
ENDIF ()
# vim: ts=2 sw=2 et:
