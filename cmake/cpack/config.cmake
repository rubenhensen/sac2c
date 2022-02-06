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
  SET (CPACK_GENERATOR "productbuild;TGZ")
ELSE ()
  SET (CPACK_GENERATOR "RPM;DEB;TGZ")
ENDIF ()

SET (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
SET (CPACK_RPM_COMPONENT_INSTALL ON)
SET (CPACK_DEB_COMPONENT_INSTALL ON)
SET (CPACK_Bundle_COMPONENT_INSTALL ON)
SET (CPACK_productbuild_COMPONENT_INSTALL ON)
SET (CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

# We create separate config files for different generators
SET (CPACK_PROJECT_CONFIG_FILE "${SAC2C_SOURCE_DIR}/cmake/cpack/options.cmake")

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
SET (CPACK_PACKAGE_VERSION_MAJOR     "${SAC2C_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR     "${SAC2C_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH     "${SAC2C_VERSION_PATCH}")
SET (CPACK_PACKAGE_FILE_NAME         "sac2c-${SAC2C_VERSION}-${PACKAGE_POSTFIX}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "sac2c-${SAC2C_VERSION}") # XXX is this really needed?
SET (CPACK_PACKAGE_ICON              "${SAC2C_SOURCE_DIR}/cmake/cpack/sac_logo.png")

# SET (CPACK_PACKAGE_DESCRIPTION_FILE ...)
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is the compiler for the data-parallel array-based functional language SaC")
# FIXME(artem) We need to decide on where do we put the stuff on the target system...)
#SET (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET (CPACK_RESOURCE_FILE_LICENSE "${SAC2C_SOURCE_DIR}/LICENSE.md")
SET (CPACK_RESOURCE_FILE_README "${SAC2C_SOURCE_DIR}/cmake/cpack/README.txt")
SET (CPACK_RESOURCE_FILE_WELCOME "${SAC2C_SOURCE_DIR}/cmake/cpack/WELCOME.txt")

# Set the displayed names for each of the components to install.
# These will be displayed in the list of components inside the installer.
SET (CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "SaC Binaries")
SET (CPACK_COMPONENT_RTAPPLICATIONS_DISPLAY_NAME "SaC Runtime Binaries")
SET (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "SaC Libraries")
SET (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "SaC Headers")
SET (CPACK_COMPONENT_CONFIG_DISPLAY_NAME "SaC Configs")
SET (CPACK_COMPONENT_SOURCES_DISPLAY_NAME "SaC Binary Sources")
SET (CPACK_COMPONENT_SYMLINKS_DISPLAY_NAME "SaC Symlinks")

# Debian-specific variables
SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "gcc, libc6 (>= 2.13), uuid-runtime (>= 2.20), libhwloc-dev")

# RPM-specific variables
# XXX (hans): this may not be exhaustive - does not take into account if the user
# changes the install prefix
SET (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/local /usr/local/bin /usr/local/include /usr/local/lib /usr/local/libexec /usr/local/share)
SET (CPACK_RPM_PACKAGE_AUTOREQ OFF) # XXX disable requires for the moment
# FIXME Can we auto-generate these dependencies?
SET (CPACK_RPM_PACKAGE_REQUIRES "gcc, hwloc-devel, libuuid-devel")
# rpmbuild tools is very smart, but also not smart enough. It knows
# that sac2c has dependencies on libsac_p and libsac_d, but figure out that
# SaC provides these, as such we get dependency issues. The real reason is
# that rpmbuild does not look under subdirs of /lib/ as such all runtime
# libraries are missed in the search. FIXME the current solution is a bandaid
# and should be replaced with something dynamic - one idea is to inject RPM SPEC
# file commands into CPACK_RPM_PACKAGE_PROVIDES, of the form `%(echo ${LOC_OF LIBS} | /usr/lib/rpm/rpmdeps -P)` which will generate the dependencies for us.
# This needs to be tested though first!!!
SET (_RPM_PROVIDES_DEBUG "libsac_d.so()(64bit), libsacdistmem_d.so()(64bit), libsac_d.so()(64bit), libsacphmc.diag_d.so()(64bit), libsacphmc_d.so()(64bit), libsacphm.diag_d.so()(64bit), libsacphm_d.so()(64bit), libsacrtspec_d.so()(64bit)")
SET (_RPM_PROVIDES_RELEASE "libsac_p.so()(64bit), libsacdistmem_p.so()(64bit), libsac_p.so()(64bit), libsacphmc.diag_p.so()(64bit), libsacphmc_p.so()(64bit), libsacphm.diag_p.so()(64bit), libsacphm_p.so()(64bit), libsacrtspec_p.so()(64bit)")
IF ("${PACKAGE_POSTFIX}" STREQUAL "omnibus")
  SET (CPACK_RPM_PACKAGE_PROVIDES "${_RPM_PROVIDES_DEBUG}, ${_RPM_PROVIDES_RELEASE}")
ELSEIF ("${PACKAGE_POSTFIX}" STREQUAL "debug")
  SET (CPACK_RPM_PACKAGE_PROVIDES "${_RPM_PROVIDES_DEBUG}")
ELSE ()
  SET (CPACK_RPM_PACKAGE_PROVIDES "${_RPM_PROVIDES_RELEASE}")
ENDIF ()

# Disable CPack if we are dirty
IF (NOT SAC2C_IS_DIRTY)
  INCLUDE (CPack)
ENDIF ()
# vim: ts=2 sw=2 et:
