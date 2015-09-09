CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (sac C)

# SET (CMAKE_VERBOSE_MAKEFILE ON)

include ("cmake/options.cmake")
include ("cmake/config.cmake")

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
  #	   instead of $SAC2CBASE/src/bin?
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
ADD_EXECUTABLE(md5 
  src/maketools/md5calc/md5.c
  src/maketools/md5calc/md5calc.c
)
BUILD_TOOL (hzip)

INCLUDE_DIRECTORIES (
  "${PROJECT_BINARY_DIR}"
  "src"
)

ADD_DEFINITIONS(
  -DPIC -D_POSIX_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -Dlint -D_POSIX_C_SOURCE=200809L 
  #${RCCCFLAGS}
  -DSANITYCHECKS 
  -DWLAA_DEACTIVATED 
  -DAE_DEACTIVATED 
  -DTSI_DEACTIVATED 
  -DPADT_DEACTIVATED 
  -DCHECK_NODE_ACCESS 
  -DINLINE_MACRO_CHECKS 
  -DSAC_BACKEND_C99 -DSAC_RC_METHOD=SAC_RCM_local
  -DSHARED_LIB_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}"
)


ADD_SUBDIRECTORY (src)

# Make sac2c binary
ADD_EXECUTABLE (sac2c src/tools/sac2c/sac2c.c)
TARGET_LINK_LIBRARIES (sac2c sacStatic ${M_PATH} ${DL_PATH} ${UUID_PATH})




# Create sac-prelude.
SET (PRELUDE_OUT   "lib${SAC_PRELUDE_NAME}Tree.so" 
		   "lib${SAC_PRELUDE_NAME}Mod.so"
		   "lib${SAC_PRELUDE_NAME}Mod.a")
SET (PRELUDE_IN	   
     "${PROJECT_SOURCE_DIR}/src/lib${SAC_PRELUDE_NAME}/${SAC_PRELUDE_NAME}.sac")

ADD_CUSTOM_COMMAND (
  WORKING_DIRECTORY   "${PROJECT_SOURCE_DIR}/lib"
  OUTPUT    ${PRELUDE_OUT}
  COMMAND   "sac2c" -v0 -O3 -noprelude ${PRELUDE_IN}
  DEPENDS   sac2c sac2cShared ${PRELUDE_IN}
)
ADD_CUSTOM_TARGET (sacprelude ALL DEPENDS ${PRELUDE_OUT})

