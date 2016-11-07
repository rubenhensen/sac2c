CMAKE_MINIMUM_REQUIRED (VERSION 3.4)

# Name of the project
PROJECT (sac2c C)

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
INCLUDE (FeatureSummary)

# Include config related cmake files
include ("cmake/macros.cmake")
include ("cmake/options.cmake")
include ("cmake/sac2c/config.cmake")
include ("cmake/sac2c/sac2crc.cmake")
include ("cmake/settings.cmake")

# If the status of git repository is "dirty", installation targets should
# be not available.
STRING (FIND "${SAC2C_VERSION}" "dirty" GITDIRTY)
IF (GITDIRTY GREATER 0) # If 'dirty' not found, we are -1, otherwise something
                        # greater than 0.
    MACRO (XINSTALL)
    ENDMACRO ()
    INSTALL(CODE 
        "EXECUTE_PROCESS (
            COMMAND ${CMAKE_COMMAND} -E echo 
                \"\n\n The current build includes local changes that are\n\"
                \"not committed to the git repository, as indicated by\n\"
                \"git describe --dirty'. Therefore, it is NOT POSSIBLE\n\"
                \"to install the current version. Either get rid of local\n\"
                \"changes or make a commit.\n\"
         )
         MESSAGE (FATAL_ERROR \"Exiting now\")"
    )
ELSE ()
    MACRO (XINSTALL)
        INSTALL (${ARGV})
    ENDMACRO ()
ENDIF ()

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "include"
  "src/include"
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_BINARY_DIR}"
  "src"
)

ADD_FEATURE_INFO(PHM PHM "use Private Heap Manager in sac2c")
ADD_FEATURE_INFO(LPEL LPEL "use LPEL in sac2c")
ADD_FEATURE_INFO(OMP OMP "use OpenMP backend for sac2c multithreading")
ADD_FEATURE_INFO(MT MT "use POSIX-thread backend for sac2c multithreading")
ADD_FEATURE_INFO(LTO LTO "adds Link Time Optimisations to the C compiler")
ADD_FEATURE_INFO(Dot DOT "used to generate a visual of the AST")
ADD_FEATURE_INFO(C++ CPLUSPLUS "to compile sac2c with")
FEATURE_SUMMARY(WHAT ALL)

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

# We create a state file of the GIT repo and use it
# to initiat automatic re-configuration of the build
# system.
# FIXME (hans): we need a way to have the flags in
#               one place, and not here, the script,
#               and config.cmake...
EXECUTE_PROCESS (
    COMMAND
      ${GIT_EXECUTABLE} describe --tags --abbrev=4 --dirty
    OUTPUT_FILE
      "${PROJECT_BINARY_DIR}/__version-git.txt"
)
CONFIGURE_FILE ("${PROJECT_BINARY_DIR}/__version-git.txt" "${PROJECT_BINARY_DIR}/__version-git.txt" COPYONLY)
# XXX We add a target to the build system that *MUST* dependency
# of all other targets relating to the GIT repo source files.
ADD_CUSTOM_TARGET (check_git_version
    COMMAND
      ${CMAKE_COMMAND} 
        -D GIT_COMMAND="${GIT_EXECUTABLE}"
        -D BUILD_DIR="${PROJECT_BINARY_DIR}"
        -P "${PROJECT_SOURCE_DIR}/cmake/check-git-version.cmake"
    BYPRODUCTS
      "${PROJECT_BINARY_DIR}/__version-git.txt"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Checking GIT repo version"
)

# generate files necessary to compile sac2c
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xsacdirs.h.in" "${PROJECT_BINARY_DIR}/include/sacdirs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in" "${PROJECT_BINARY_DIR}/src/build.c")

# Build sac.h
ADD_SUBDIRECTORY (src/runtime)
ADD_DEPENDENCIES (sac_h check_git_version)

# Build libsac2c
ADD_SUBDIRECTORY (src/libsac2c)
ADD_DEPENDENCIES (sac2cShared check_git_version)

# Make sac2c-related binaryies
ADD_EXECUTABLE (sac2c src/tools/sac2c/sac2c.c)
ADD_DEPENDENCIES (sac2c sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

ADD_EXECUTABLE (sac4c src/tools/sac4c/sac4c.c)
ADD_DEPENDENCIES (sac4c sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac4c ${DL_LIB})

ADD_EXECUTABLE (sac2tex src/tools/sac2tex/sac2tex.c)
ADD_DEPENDENCIES (sac2tex sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac2tex ${DL_LIB})

# XXX(artem) This is an alternative way to acheive what ExternalProject +
#            ExternalProject_AddStep are doing.
#ADD_CUSTOM_TARGET (
#    build-runtime-libs ALL
#    DEPENDS
#        sac2c sac2cShared
#    COMMAND
#        ${CMAKE_COMMAND} -E make_directory runtime_build
#    COMMAND
#        ${CMAKE_COMMAND} 
#            -E chdir runtime_build
#            ${CMAKE_COMMAND}
#                -DSAC2C_BUILD_DIR=${PROJECT_BINARY_DIR}
#                -DSAC2C_SOURCE_DIR=${PROJECT_SOURCE_DIR}
#                -DSAC2CRC_BUILD_CONF=${SAC2CRC_BUILD_CONF}
#                # FIXME(artem) Shouldn't we use MODEXT from sac2crc instead?
#                -DSHARED_LIB_EXT=${SHARED_LIB_EXT}
#                -DLIB_RT=${LIB_RT}
#                ${PROJECT_SOURCE_DIR}/cmake/runtime
#    COMMAND
#        ${CMAKE_COMMAND} --build runtime_build
#    COMMENT
#        "Build runtime sac2c libraries for targets: ${targets}"
#)

# This is where we call the build of the sac2c shared-libraries
# XXX: this *depends* on sac2c having been build first!
ExternalProject_Add(runtime_libraries
    DEPENDS sac2cShared sac2c check_git_version
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    INSTALL_COMMAND "" # this is to prevent any install target from being called
    PREFIX runtime_build
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/cmake/runtime
    INSTALL_DIR ${PROJECT_BINARY_DIR} #XXX: this is not used...
    CMAKE_ARGS
        -DSAC2C_BUILD_DIR=${PROJECT_BINARY_DIR}
        -DSAC2C_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DSAC2CRC_BUILD_CONF=${SAC2CRC_BUILD_CONF}
        # FIXME(artem) Shouldn't we use MODEXT from sac2crc instead?
        -DSHARED_LIB_EXT=${SHARED_LIB_EXT}
        -DLIB_RT=${LIB_RT}
)

# Force runtime_libraries to reconfigure every time we run make.
ExternalProject_Add_Step(runtime_libraries forceconfigure
            COMMAND ${CMAKE_COMMAND} -E echo "Force configure of runtime libraries"
            DEPENDEES update
            DEPENDERS configure
            ALWAYS 1)


# Installing the sac2c project files.
# XXX(artem) in case we are doing packaging, symbolic links should be
#            avoided.
XINSTALL (TARGETS sac2c RUNTIME DESTINATION "${DLL_DIR}" COMPONENT applications)
IF (CMAKE_HOST_UNIX AND SYMLINKS)
    # FIXME(artem) Create symlinks for other tools we install
    # A better way to create symlinks is:
    #    INSTALL(CODE "
    #              EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E create_symlink
    #              ...
    #              )
    #    ")
    XINSTALL (CODE "
        FILE (MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
        EXECUTE_PROCESS (
            COMMAND ln -s ${DLL_DIR}/sac2c sac2c
            WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
    ")
ELSE ()
    XINSTALL (TARGETS sac2c RUNTIME DESTINATION bin COMPONENT applications)
    XINSTALL (TARGETS sac4c RUNTIME DESTINATION bin COMPONENT applications)
    XINSTALL (TARGETS sac2tex RUNTIME DESTINATION bin COMPONENT applications)
ENDIF ()

# install sac2crc stuff...
XINSTALL (FILES "${SAC2CRC_BUILD_CONF}"
         DESTINATION "${SAC2CRC_DIR}"
         COMPONENT config)

# install sac2c-related binaries stuff...
XINSTALL (FILES "${PROJECT_BINARY_DIR}/saccc"
         DESTINATION bin 
         COMPONENT applications)

# install include files
XINSTALL (DIRECTORY "${PROJECT_SOURCE_DIR}/include/" 
         DESTINATION "${INCPATH_CONF}"
         COMPONENT headers)

## We need to install the runtime libraries and sacprelude into a directory
## hiearchy that is different from the `local' hiearchy. Under out local `lib/' we
## have the 'prelude' directory and then one or more other directories - these are
## the 'TARGET_ENV' or target environment specific libries, e.g. 'x64' is for AMD64
## system and 'x86' is for i386 systems. How these directories are named can not be
## known before hand, as this is defined through that target within the sac2crc
## file. Because of this, we use pattern matching to select the correct directories
## to be installed in the new heirchy. The pattern match is a generic start (*)
## glob, which means we get all directories within `lib' - including `prelude'. To
## avoid installing prelude, we have use the EXCLUDE directive with the pattern
## "prelude". We do the same for the local directory `lib/prelude', but exlude
## 'tree' instead.
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/
    DESTINATION ${RTPATH_CONF}
    COMPONENT libraries
    FILES_MATCHING PATTERN "*"
    PATTERN "prelude" EXCLUDE)
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/prelude/
    DESTINATION ${MODPATH_CONF}
    COMPONENT libraries
    FILES_MATCHING PATTERN "*"
    PATTERN "tree" EXCLUDE)

# install runtime-related binaries
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/bin/
         DESTINATION bin
         COMPONENT applications
         FILES_MATCHING PATTERN "*")

# install sacprelude
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/prelude/tree
         DESTINATION ${TREEPATH_CONF}
         COMPONENT libraries)

# Cpack information
# INCLUDE(InstallRequiredSystemLibraries)

# By setting this on we can see where installation targets are specified via
# absolute pathes.  For portability pruroses this should be avoided.
# SET (CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION ON)

SET (CPACK_GENERATOR "TGZ")
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET (CPACK_GENERATOR "Bundle")
ENDIF()
    
# XXX(artem) This allows one to use absolute pathes during installation,
#            but it doesn't seem to work on windows and friends.
SET (CPACK_SET_DESTDIR ON)
SET (CPACK_PACKAGE_NAME              "sac2c-compiler")
SET (CPACK_PACKAGE_VENDOR            "SaC Development Team")
SET (CPACK_PACKAGE_VERSION           "${SAC2C_VERSION}")
SET (CPACK_PACKAGE_VERSION_MAJOR     "${SAC2C_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR     "${SAC2C_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH     "${SAC2C_VERSION_PATCH}")
SET (CPACK_PACKAGE_FILE_NAME         "sac2c-${SAC2C_VERSION}-${CMAKE_BUILD_TYPE}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "sac2c-${SAC2C_VERSION}")
SET (CPACK_PACKAGE_ICON              "${PROJECT_SOURCE_DIR}/cmake/cpack/dmg-bundle/sac_logo.png")

# DMG Bundle related configuration
SET (CPACK_BUNDLE_NAME "sac2c-${SAC2C_VERSION}-${CMAKE_BUILD_TYPE}")
## Use Macports 'makeicns' tool to generate MAC OS X icons 
SET (CPACK_BUNDLE_ICON "${PROJECT_SOURCE_DIR}/cmake/cpack/dmg-bundle/sac.icns")
SET (CPACK_BUNDLE_PLIST "${PROJECT_BINARY_DIR}/Info.plist")
SET (CPACK_BUNDLE_STARTUP_COMMAND "${PROJECT_BINARY_DIR}/Install.command")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/cmake/cpack/dmg-bundle/Info.plist.in" "${PROJECT_BINARY_DIR}/Info.plist")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/cmake/cpack/dmg-bundle/Install.command.in" "${PROJECT_BINARY_DIR}/Install.command")

# SET (CPACK_PACKAGE_DESCRIPTION_FILE ...)
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "The sac2c compiler for a data-parallel array-based functional language SAC")
# FIXME(artem) We need to decide on where do we put the stuff on the target system...)
#SET (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

# Debian-specific variables
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "SAC Development Team <info@sac-home.org>")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# RPM-specific variables
# FIXME(hans) this may not be exhaustive - does not take into account if the user
# changes the install prefix
SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/local" "/usr/local/include" "/usr/local/lib" "/usr/local/libexec" "/usr/local/share")

SET (CPACK_COMPONENTS_ALL applications libraries headers conf)
# Set the displayed names for each of the components to install.
# These will be displayed in the list of components inside the installer.
SET (CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "SaC Binaries")
SET (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "SaC Libraries")
SET (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "SaC Headers")
SET (CPACK_COMPONENT_CONF_DISPLAY_NAME "SaC Configs")

INCLUDE(CPack)
