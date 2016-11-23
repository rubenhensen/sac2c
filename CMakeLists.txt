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
    SET(SAC2C_IS_DIRTY 1)
    MACRO (XINSTALL)
    ENDMACRO ()
    INSTALL(CODE 
        "EXECUTE_PROCESS (
            COMMAND ${CMAKE_COMMAND} -E echo 
                \"\n\n The current build includes local changes that are\n\"
                \"not committed to the git repository, as indicated by\n\"
                \"'git describe --dirty'. Therefore, it is NOT POSSIBLE\n\"
                \"to install the current version. Either get rid of local\n\"
                \"changes or make a commit.\n\"
         )
         MESSAGE (FATAL_ERROR \"Exiting now\")"
    )
ELSE ()
    SET(SAC2C_IS_DIRTY 0)
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

# Make sac2c-related binaries
ADD_EXECUTABLE (sac2c "${PROJECT_SOURCE_DIR}/src/tools/sac2c/sac2c.c")
ADD_DEPENDENCIES (sac2c sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

ADD_EXECUTABLE (sac4c "${PROJECT_SOURCE_DIR}/src/tools/sac4c/sac4c.c")
ADD_DEPENDENCIES (sac4c sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac4c ${DL_LIB})

ADD_EXECUTABLE (sac2tex "${PROJECT_SOURCE_DIR}/src/tools/sac2tex/sac2tex.c")
ADD_DEPENDENCIES (sac2tex sac_h check_git_version)
TARGET_LINK_LIBRARIES (sac2tex ${DL_LIB})

# Set the postfix of the executables depending on build type
SET_TARGET_PROPERTIES (sac2c sac4c sac2tex
    PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
               RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})

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
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_RELEASE_POSTFIX=${CMAKE_RELEASE_POSTFIX}
        -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
        -DBUILD_TYPE_POSTFIX=${BUILD_TYPE_POSTFIX}
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
XINSTALL (TARGETS sac2c RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)
XINSTALL (TARGETS sac4c RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)
XINSTALL (TARGETS sac2tex RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)

# install sac2crc stuff...
XINSTALL (FILES "${SAC2CRC_BUILD_CONF}"
  DESTINATION "${SAC2CRC_INSTALL}"
  COMPONENT config)

# install sac2c-related binaries stuff...
XINSTALL (PROGRAMS "${PROJECT_BINARY_DIR}/saccc"
  DESTINATION "${TREEPATH_INSTALL}"
  RENAME "saccc${BUILD_TYPE_POSTFIX}"
  COMPONENT applications)

# install include files
XINSTALL (DIRECTORY "${PROJECT_SOURCE_DIR}/include/" 
  DESTINATION "${INCPATH_INSTALL}"
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
  DESTINATION "${RTPATH_INSTALL}"
  COMPONENT libraries
  FILES_MATCHING PATTERN "*"
  PATTERN "prelude" EXCLUDE)

XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/prelude/
  DESTINATION "${MODPATH_INSTALL}"
  COMPONENT libraries
  FILES_MATCHING PATTERN "*"
  PATTERN "tree" EXCLUDE)

# install runtime-related binaries
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/bin/
  DESTINATION "${TREEPATH_INSTALL}"
  COMPONENT rtapplications
  FILES_MATCHING PATTERN "*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE)

# install sacprelude
XINSTALL (DIRECTORY ${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/prelude/tree
  DESTINATION "${TREEPATH_INSTALL}"
  COMPONENT libraries)
  
# we install sources of binaries
XINSTALL (FILES
    ${PROJECT_SOURCE_DIR}/src/tools/sac2c/sac2c.c
    ${PROJECT_SOURCE_DIR}/src/tools/sac2tex/sac2tex.c
    ${PROJECT_SOURCE_DIR}/src/tools/sac4c/sac4c.c
    ${PROJECT_SOURCE_DIR}/src/include/sactools.h
    ${PROJECT_BINARY_DIR}/include/sacdirs.h
  DESTINATION src
  COMPONENT sources)

# we install readme and install script
XINSTALL (FILES
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/README
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/install.sh
  DESTINATION "."
  COMPONENT sources)

# Cpack information
# INCLUDE(InstallRequiredSystemLibraries)

# By setting this on we can see where installation targets are specified via
# absolute pathes.  For portability pruroses this should be avoided.
SET (CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION ON)

#SET (CPACK_GENERATOR "TGZ")
#IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    SET (CPACK_GENERATOR "Bundle")
#ENDIF()
SET(SAC2C_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

# now lets setup packaging (using CPack)
INCLUDE ("${PROJECT_SOURCE_DIR}/cmake/cpack/config.cmake")

# vim: ts=2 sw=2 et:
