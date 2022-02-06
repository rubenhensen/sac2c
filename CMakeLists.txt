CMAKE_MINIMUM_REQUIRED (VERSION 3.17)

# Name of the project
PROJECT (sac2c)

# CMake policy handling
IF (POLICY CMP0075)
  # CMAKE_REQUIRED_LIBRARIES usage for CHECK_INCLUDE_FILE(S) macro
  CMAKE_POLICY (SET CMP0075 NEW)
ENDIF ()

IF (POLICY CMP0114)
  # ExternalProject_Add() dependency handling
  CMAKE_POLICY (SET CMP0114 OLD)
ENDIF ()

# Extra cmake packages
INCLUDE (ExternalProject)
INCLUDE (FeatureSummary)

# DEFAULT Build Targets for Runtime Libraries
SET (RT_TARGETS seq seq_rt mt_pth mt_pth_rt mt_pth_xt)

# Include config related cmake files
SET(SAC2C_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
INCLUDE ("cmake/macros.cmake")
INCLUDE ("cmake/options.cmake")
INCLUDE ("cmake/sac2c/config.cmake")

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
ADD_FEATURE_INFO(ISL ISL "use Integer Set Library in sac2c")
ADD_FEATURE_INFO(BARVINOK BARVINOK "use Barvinok Library in sac2c")
ADD_FEATURE_INFO(FUNCTESTS FUNCTESTS
                 "enable functional tests (gtest is required)")
ADD_FEATURE_INFO(BUILDGENERIC BUILDGENERIC
                 "build the compiler with -mtune=generic")
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
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/tools/fake-cc.in" "${PROJECT_BINARY_DIR}/fake-cc" @ONLY)

# generate files necessary to compile sac2c
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/fun-attrs.h.in" "${PROJECT_BINARY_DIR}/include/fun-attrs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xsacdirs.h.in" "${PROJECT_BINARY_DIR}/include/sacdirs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in" "${PROJECT_BINARY_DIR}/src/build.c")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/override_cuda_runtime.h" "${PROJECT_BINARY_DIR}/include/override_cuda_runtime.h" COPYONLY)

# Build sac.h
ADD_SUBDIRECTORY (src/runtime)
ADD_DEPENDENCIES (sac_h check_repo_version)

# Build libsac2c
ADD_SUBDIRECTORY (src/libsac2c)
ADD_DEPENDENCIES (sac2cShared check_repo_version)

# Make sac2c-related binaries
ADD_EXECUTABLE (sac2c "${PROJECT_SOURCE_DIR}/src/tools/sac2c/sac2c.c")
ADD_DEPENDENCIES (sac2c check_repo_version)
TARGET_LINK_LIBRARIES (sac2c ${DL_LIB})

ADD_EXECUTABLE (sac4c "${PROJECT_SOURCE_DIR}/src/tools/sac4c/sac4c.c")
ADD_DEPENDENCIES (sac4c check_repo_version)
TARGET_LINK_LIBRARIES (sac4c ${DL_LIB})

ADD_EXECUTABLE (sac2tex "${PROJECT_SOURCE_DIR}/src/tools/sac2tex/sac2tex.c")
ADD_DEPENDENCIES (sac2tex check_repo_version)
TARGET_LINK_LIBRARIES (sac2tex ${DL_LIB})

IF (ENABLE_ISL AND ENABLE_BARVINOK)
  SET_SOURCE_FILES_PROPERTIES(
    src/tools/sacislinterface/sacislinterface.c
    PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-conversion"
  )
  ADD_EXECUTABLE (sacislinterface src/tools/sacislinterface/sacislinterface.c)
  ADD_DEPENDENCIES (sacislinterface check_repo_version)
  TARGET_LINK_LIBRARIES (sacislinterface ${LIB_ISL} ${LIB_BARVINOK} ${BARVINOK_LIB})
  TARGET_INCLUDE_DIRECTORIES (sacislinterface PUBLIC ${BARVINOK_INC_PATH} ${ISL_INC_PATH})
ENDIF ()


# Set the postfix of the executables depending on build type
SET_TARGET_PROPERTIES (sac2c sac4c sac2tex
    PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
               RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})

# This target will cause all build dirs (including externalprojects) to be cleaned
ADD_CUSTOM_TARGET(fullclean
  COMMAND
    +${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target clean
  COMMENT "Cleaning sac2c and runtime libraries")

# we need to convert the target `list' into a string in order to pass it to the
# runtime build system - we convert it back into a list on the other end.
STRING (REPLACE ";" ":" _TARGETS "${RT_TARGETS}")

# This is where we call the build of the sac2c shared-libraries
# This *depends* on sac2c having been build first!
ExternalProject_Add(runtime_libraries
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
        -DLINKSETSIZE=${LINKSETSIZE}
        -DRT_TARGETS:STRING=${_TARGETS}
        -DBUILDGENERIC=${BUILDGENERIC}
)
# We set dependencies on the configure step, this makes sure we propogate
# certain values (such as from sac2crc).
ExternalProject_Add_StepDependencies(runtime_libraries configure sac2c sac2cShared sac_h check_repo_version)
# Get runtime library build directory
ExternalProject_Get_Property (runtime_libraries BINARY_DIR)
SET (RUNTIME_BINARY_DIR ${BINARY_DIR}) # redefine name to something more useful
UNSET (BINARY_DIR) # make sure we don't cause a conflict
# Create a step to clean the build directory
ExternalProject_Add_Step (runtime_libraries clean
  COMMAND
  +${CMAKE_COMMAND} --build ${RUNTIME_BINARY_DIR} --target clean
  DEPENDEES configure
  COMMENT "Cleaning runtime libraries, in ${RUNTIME_BINARY_DIR}"
  EXCLUDE_FROM_MAIN 1
  ALWAYS 1)
# create both the local sac2crc file and the prelude sac2crc file
INCLUDE ("cmake/sac2c/sac2crc.cmake")
# Create a target which links to the clean step
ExternalProject_Add_StepTargets (runtime_libraries clean)
# Add the target as a dependency to the `fullclean' target
ADD_DEPENDENCIES (fullclean runtime_libraries-clean)
# We extend ExternalProject definition to include targets to directly
# build specifc libraries (target specific), additionally we include some
# properties in the target to make it easier to use.
# XXX (hans) we are not creating a library target, meaning it can no be used
# directly as part of e.g. target_link_libraries functions. As such we instead
# force add target properties which we can access explicitly.
# Expose configure step as target
# XXX (hans) additionaly we explicitly set inter-step dependencies as this
# facility is broken in cmake >= version 3.10,
# - see https://gitlab.kitware.com/cmake/cmake/issues/18663
ExternalProject_Add_StepTargets (runtime_libraries configure build install)
ExternalProject_Add_StepDependencies (runtime_libraries install runtime_libraries-build)
ExternalProject_Add_StepDependencies (runtime_libraries build runtime_libraries-configure)
FOREACH (__target ${RT_TARGETS})
  ADD_CUSTOM_TARGET (libsac-${__target}
    COMMAND +${CMAKE_COMMAND} --build ${RUNTIME_BINARY_DIR} --target libsac-${__target}
  )
  ADD_DEPENDENCIES (libsac-${__target} runtime_libraries-configure)
  STRING (REPLACE "_" "-"  __sbi "${__target}") # FIXME not consistent
  SET_TARGET_PROPERTIES (libsac-${__target}
    PROPERTIES
    OUTPUT_NAME "${RUNTIME_BINARY_DIR}/lib/host/${__sbi}/libsac${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}"
    BUILD_RPATH "${RUNTIME_BINARY_DIR}/lib/host/${__sbi}")
ENDFOREACH ()

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
ADD_INSTALL_TARGET ("headers" sac_h)
ADD_INSTALL_TARGET ("libraries" runtime_libraries)
ADD_INSTALL_TARGET ("rtapplications" runtime_libraries)
ADD_INSTALL_TARGET ("sources")
ADD_INSTALL_TARGET ("symlinks")

# Installing the sac2c project files.
XINSTALL (TARGETS sac2c RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)
XINSTALL (TARGETS sac4c RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)
XINSTALL (TARGETS sac2tex RUNTIME DESTINATION "${TREEPATH_INSTALL}" COMPONENT applications)

# install sac2crc stuff...
XINSTALL (FILES "${SAC2CRC_BUILD_CONF}"
  DESTINATION "${SAC2CRC_INSTALL}"
  COMPONENT config)

# install sac2c-related binaries stuff...
# FIXME (hans) incorrect component - where do we use saccc again?
XINSTALL (PROGRAMS "${PROJECT_BINARY_DIR}/saccc"
  DESTINATION "${TREEPATH_INSTALL}"
  RENAME "saccc${BUILD_TYPE_POSTFIX}"
  COMPONENT rtapplications)

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

# install further includes
XINSTALL (
  DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)

# and also the build config
XINSTALL (
  FILES "${PROJECT_BINARY_DIR}/include/config.h" "${PROJECT_BINARY_DIR}/include/fun-attrs.h"
        "${PROJECT_BINARY_DIR}/include/override_cuda_runtime.h"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)

# we install sources of binaries
XINSTALL (FILES
    ${PROJECT_SOURCE_DIR}/src/tools/sac2c/sac2c.c
    ${PROJECT_SOURCE_DIR}/src/tools/sac2tex/sac2tex.c
    ${PROJECT_SOURCE_DIR}/src/tools/sac4c/sac4c.c
    ${PROJECT_SOURCE_DIR}/src/include/sactools.h
    ${PROJECT_BINARY_DIR}/include/sacdirs.h
  DESTINATION "${SOURCE_INSTALL}"
  COMPONENT sources
  EXCLUDE_FROM_ALL)

# we install readme and install script
CONFIGURE_FILE (${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/installer.sh.in
                ${PROJECT_BINARY_DIR}/archive/installer-${BUILD_TYPE_NAME}.sh @ONLY)
XINSTALL (FILES
    ${PROJECT_BINARY_DIR}/archive/installer-${BUILD_TYPE_NAME}.sh
  DESTINATION "${INSTALLER_INSTALL}"
  COMPONENT sources
  EXCLUDE_FROM_ALL)

XINSTALL (FILES
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/install.sh
    ${PROJECT_SOURCE_DIR}/cmake/cpack/dists/archive/README
  DESTINATION "${TOP_INSTALL}"
  COMPONENT sources
  EXCLUDE_FROM_ALL)

# (hans) NOTE: We need to create the `_top_install` and `_bin_install` variables
# for when we install symlinks using `INSTALL (CODE ...)`. These two variables
# are based upon the `TOP_INSTALL` and `BIN_INSTALL` variables found in
# sac2c/config.cmake - these variables are automatically generated and are
# prepended with a ``package prefix'' when using CPack directly. The `*_INSTALL`
# variables are relative paths on where to install the generated SAC binaries
# and libraries during the install stage of the build system. The `_*_install`
# variables are only needed for the symlinks because we are using the `INSTALL
# (CODE ...)` function instead of the `INSTALL(... DESTINATION ...)` function.
# In the latter's case, internally any relative path given as a parameter to
# DESTINATION is prepended with CMAKE_INSTALL_PREFIX (and also `ENV(DESTDIR)`).
# This is so that the install stage can correctly place the resulting
# files/dirs. `DESTDIR` is set via `make DESTDIR=... install`, typically only
# called by CPack packagers that don't support directly changing
# CMAKE_INSTALL_PREFIX, or defining their own pakaging prefix via
# `PACKAGE_PREFIX...` (see sac2c/config.cmake). In our case, because we are
# using a CODE block to run the sac2c-version-manager script, this internal
# facility is not given. The generation of the two variables mimics the internal
# action and allows for the script to be prepended with CMAKE_INSTALL_PREFIX.
IF (NOT IS_ABSOLUTE "${BIN_INSTALL}")
  # We escape ${CMAKE_INSTALL_PREFIX} because the install stage resolves this
  # variable independently of the configure stage - this allows CPack and other
  # tools to affect where files/dirs are installed too.
  SET (_bin_install "\\\$ENV{DESTDIR}\\\${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL}")
ELSE ()
  SET (_bin_install "${BIN_INSTALL}")
ENDIF ()

IF (NOT IS_ABSOLUTE "${TOP_INSTALL}")
  # We escape ${CMAKE_INSTALL_PREFIX} because the install stage resolves this
  # variable independently of the configure stage - this allows CPack and other
  # tools to affect where files/dirs are installed too.
  SET (_top_install "\\\${CMAKE_INSTALL_PREFIX}/${TOP_INSTALL}")
ELSE ()
  SET (_top_install "${TOP_INSTALL}")
ENDIF ()

# We use the sac2c-version-manager script instead of the CMAKE's facility to
# create symlinks because the scripts does additional checks to make sure
# that the links are sanely done and don't accidentally overwrite anything
# that shouldn't be overwritten.
XINSTALL (
  CODE "EXECUTE_PROCESS(COMMAND
    ${PROJECT_BINARY_DIR}/scripts/sac2c-version-manager install
      \"${SAC2C_VERSION}\" \"${CMAKE_BUILD_TYPE}\"
      \"${_top_install}\" \"${_bin_install}\")"
  COMPONENT symlinks)

# now lets setup packaging (using CPack)
INCLUDE ("${PROJECT_SOURCE_DIR}/cmake/cpack/config.cmake")

# Unit Testing and related
ENABLE_TESTING ()
# This target exists because the `test` target of CMake is not visible
# within a CMakeLists.txt config. One cannot add target dependencies to
# it, so instead we create our own target. This mimicks autotools' `check`
# Makefile target. By default, we depend on the compiler being built.
# NOTE: in general, we should only add dependencies which are (a) nessicary
#       files for a test, (b) executables that need to be built. Additionally
#       we need the sac2crc generation target, as otherwise there could be
#       issues with finding the correct sacprelude.
ADD_CUSTOM_TARGET (check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --output-junit sac2c_${CMAKE_BUILD_TYPE}_tests.xml
  DEPENDS copy-sac2crc-prelude-file runtime_libraries)

# The following director contains all tests which check that the compiler
# is working correctly, e.g. code transformations make the expected changes,
# options flags work (features are working), etc.
ADD_SUBDIRECTORY ("tests")

# This diectory contains all tests the check the compiler at the function level,
# that modules and other compilation units are working. We by default have this
# collection of units tests disabled, as it relies on GTest. GTest needs to be
# installed by the user themselves.
IF (FUNCTESTS)
  ADD_SUBDIRECTORY ("src/tests")
ENDIF ()

# vim:ts=2:sw=2:et:
