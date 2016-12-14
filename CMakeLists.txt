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
INCLUDE ("cmake/macros.cmake")
INCLUDE ("cmake/options.cmake")
INCLUDE ("cmake/sac2c/config.cmake")
INCLUDE ("cmake/settings.cmake")

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

# generate files necessary to compile sac2c
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xsacdirs.h.in" "${PROJECT_BINARY_DIR}/include/sacdirs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in" "${PROJECT_BINARY_DIR}/src/build.c")

# Build sac.h
ADD_SUBDIRECTORY (src/runtime)
ADD_DEPENDENCIES (sac_h check_repo_version)

# Build libsac2c
ADD_SUBDIRECTORY (src/libsac2c)
ADD_DEPENDENCIES (sac2cShared check_repo_version)

# Make sac2c-related binaries
ADD_EXECUTABLE (sac2c "${PROJECT_SOURCE_DIR}/src/tools/sac2c/sac2c.c")
ADD_DEPENDENCIES (sac2c sac_h check_repo_version)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

ADD_EXECUTABLE (sac4c "${PROJECT_SOURCE_DIR}/src/tools/sac4c/sac4c.c")
ADD_DEPENDENCIES (sac4c sac_h check_repo_version)
TARGET_LINK_LIBRARIES (sac4c ${DL_LIB})

ADD_EXECUTABLE (sac2tex "${PROJECT_SOURCE_DIR}/src/tools/sac2tex/sac2tex.c")
ADD_DEPENDENCIES (sac2tex sac_h check_repo_version)
TARGET_LINK_LIBRARIES (sac2tex ${DL_LIB})

# Set the postfix of the executables depending on build type
SET_TARGET_PROPERTIES (sac2c sac4c sac2tex
    PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
               RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})

# This target will cause all build dirs (including externalprojects) to be cleaned
ADD_CUSTOM_TARGET(fullclean
  COMMAND
    ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target clean
  COMMENT "Cleaning sac2c and runtime libraries")

# This is where we call the build of the sac2c shared-libraries
# This *depends* on sac2c having been build first!
ExternalProject_Add(runtime_libraries
    DEPENDS sac2cShared sac2c check_repo_version
    DOWNLOAD_COMMAND "" # this is to prevent any download target from being called
    INSTALL_COMMAND "" # this is to prevent any install target from being called
    PREFIX runtime_build
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/cmake/runtime
    INSTALL_DIR ${PROJECT_BINARY_DIR} #XXX: this is not used...
    BUILD_ALWAYS 1 # this is so we react on changes within the source tree
    CMAKE_ARGS
        -DSAC2C_BUILD_DIR=${PROJECT_BINARY_DIR}
        -DSAC2C_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_TYPE_POSTFIX=${BUILD_TYPE_POSTFIX}
        -DSHARED_LIB_EXT=${SHARED_LIB_EXT}
        -DLIB_RT=${LIB_RT}
)
# We set dependencies on the configure step, this makes sure we propogate
# certain values (such as from sac2crc).
ExternalProject_Add_StepDependencies(runtime_libraries configure sac2c sac2cShared check_repo_version)
# Get runtime library build directory
ExternalProject_Get_Property (runtime_libraries BINARY_DIR STAMP_DIR)
# Create a step to clean the build directory
ExternalProject_Add_Step (runtime_libraries clean
  COMMAND
    ${CMAKE_COMMAND} --build ${BINARY_DIR} --target clean
  DEPENDEES configure
  COMMENT "Cleaning runtime libraries, in ${BINARY_DIR}"
  EXCLUDE_FROM_MAIN 1
  ALWAYS 1)
# create both the local sac2crc file and the prelude sac2crc file
SET (RUNTIME_BINARY_DIR ${BINARY_DIR})
INCLUDE ("cmake/sac2c/sac2crc.cmake")
UNSET (BINARY_DIR) # make sure we don't cause a conflict
UNSET (STAMP_DIR) # make sure we don't cause a conflict
# Create a target which links to the clean step
ExternalProject_Add_StepTargets(runtime_libraries clean)
# Add the target as a dependency to the `fullclean' target 
ADD_DEPENDENCIES (fullclean runtime_libraries-clean)

# this target is meant to copy over the sac2crc prelude file to the users
# .sac2crc directory...
# XXX This assumes that the files exists once configuration is done...
ADD_CUSTOM_TARGET (copy-sac2crc-prelude-file ALL
  COMMAND
    ${CMAKE_COMMAND}
    -DUSER_HOME="$ENV{HOME}"
    -DSAC2CRC_PATH="${PROJECT_BINARY_DIR}/sac2crc.${BUILD_TYPE_NAME}.prelude"
    -P "${PROJECT_SOURCE_DIR}/cmake/place-sac2crc-prelude-file.cmake"
  COMMENT "Copying over sac2crc file to user's .sac2crc directory")

# Create custom install-<component> target for a component provided
# as a first argument of the macro, and dependencies, pvovided as
# N arguments after the first one.
MACRO (ADD_INSTALL_TARGET)
  SET (name ${ARGV0})
  SET (deps ${ARGN})
  LIST (REMOVE_AT deps 0)
  ADD_CUSTOM_TARGET (${name}-install
    DEPENDS ${deps} 
    COMMAND
      "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${name} -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
  UNSET (name)
  UNSET (deps)
ENDMACRO ()

# Create install-<component> target for all the existing components.
ADD_INSTALL_TARGET ("applications" sac2c sac4c sac2tex "${PROJECT_BINARY_DIR}/saccc")
ADD_INSTALL_TARGET ("config" "${SAC2CRC_BUILD_CONF}")
ADD_INSTALL_TARGET ("headers" sac2cShared)
ADD_INSTALL_TARGET ("libraries" sac2cShared runtime_libraries)
ADD_INSTALL_TARGET ("rtapplications" runtime_libraries)
ADD_INSTALL_TARGET ("sources")

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

# We need to install the runtime libraries and sacprelude into a directory
# hiearchy that is different from the `local' hiearchy. Under out local `lib/' we
# have the 'prelude' directory and then one or more other directories - these are
# the 'TARGET_ENV' or target environment specific libries, e.g. 'x64' is for AMD64
# system and 'x86' is for i386 systems. How these directories are named can not be
# known before hand, as this is defined through that target within the sac2crc
# file. Because of this, we use pattern matching to select the correct directories
# to be installed in the new heirchy. The pattern match is a generic start (*)
# glob, which means we get all directories within `lib' - including `prelude'. To
# avoid installing prelude, we have use the EXCLUDE directive with the pattern
# "prelude". We do the same for the local directory `lib/prelude', but exlude
# 'tree' instead.
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
  DESTINATION "${SOURCE_INSTALL}"
  COMPONENT sources)

# we install readme and install script
CONFIGURE_FILE (${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/installer.sh.in ${PROJECT_BINARY_DIR}/archive/installer-${BUILD_TYPE_NAME}.sh @ONLY) 
XINSTALL (FILES
    ${PROJECT_BINARY_DIR}/archive/installer-${BUILD_TYPE_NAME}.sh
  DESTINATION "${INSTALLER_INSTALL}"
  COMPONENT sources)

XINSTALL (FILES
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/install.sh
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/README
  DESTINATION "${TOP_INSTALL}"
  COMPONENT sources)

# now lets setup packaging (using CPack)
SET(SAC2C_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
INCLUDE ("${PROJECT_SOURCE_DIR}/cmake/cpack/config.cmake")

# vim:ts=2:sw=2:et:
