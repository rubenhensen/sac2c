CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (sac C)

# SET (CMAKE_VERBOSE_MAKEFILE ON)

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
##ADD_EXECUTABLE(md5
##  src/maketools/md5calc/md5.c
##  src/maketools/md5calc/md5calc.c
##)
BUILD_TOOL (hzip)

INCLUDE_DIRECTORIES (
  "${PROJECT_BINARY_DIR}"
  "src"
)

ADD_SUBDIRECTORY (src)

# Make sac2c binary
ADD_EXECUTABLE (sac2c src/tools/sac2c/sac2c.c)
ADD_DEPENDENCIES (sac2c sac_h)
TARGET_LINK_LIBRARIES (sac2c ${M_LIB} ${DL_LIB} ${UUID_LIB})

#
# Create sac-prelude.
SET (PRELUDE_OUT   "lib${SAC_PRELUDE_NAME}Tree.so"
                   "lib${SAC_PRELUDE_NAME}Mod.so"
                   "lib${SAC_PRELUDE_NAME}Mod.a")
SET (PRELUDE_IN
     "${PROJECT_SOURCE_DIR}/src/lib${SAC_PRELUDE_NAME}/${SAC_PRELUDE_NAME}.sac")



# Directories to be included via -Xc and -Xtc so that sac2c could find
# sac.h and similar headers.
SET (SAC2C_EXTRA_INC "-I${PROJECT_SOURCE_DIR}/include -I${PROJECT_BINARY_DIR}/include")
ADD_CUSTOM_COMMAND (
  OUTPUT    ${PRELUDE_OUT}
  COMMAND   "sac2c" -v0 -linksetsize 0 -noprelude 
            -Xc "${SAC2C_EXTRA_INC}"
            -Xtc "${SAC2C_EXTRA_INC}"
            ${PRELUDE_IN}
  DEPENDS   sac2c sac2cShared sac2crc ${PRELUDE_IN}
)
ADD_CUSTOM_TARGET (sacprelude ALL DEPENDS ${PRELUDE_OUT})
ADD_CUSTOM_TARGET (sac2crc ALL DEPENDS "${PROJECT_SOURCE_DIR}/setup/sac2crc")


# Installing the sac2c project files.
INSTALL (TARGETS sac2c RUNTIME DESTINATION "${DLL_DIR}")
IF (CMAKE_HOST_UNIX)
    INSTALL (CODE "
                EXECUTE_PROCESS (COMMAND ln -sf sac2c \"${DLL_DIR}/sac2c\"
                WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\")
             ")
ELSE ()
    INSTALL (TARGETS sac2c RUNTIME DESTINATION bin)
ENDIF ()
# FIXME(artem) Can we do better than just copying files?
#              We have the target sacprelude...
INSTALL (FILES "${PRELUDE_OUT}" LIBRARY DESTINATION "${DLL_DIR}")
INSTALL (FILES "${SAC2CRC_BUILD_CONF}" RESOURCE DESTINATION "${SAC2CRC_DIR}")
