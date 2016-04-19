# file include checks to find sac2c, etc.

SET (ISAC2CLOCAL "${SAC2C_SOURCE_DIR}/cmake/runtime/sac2crc.local.in")
SET (LSAC2CLOCAL "${PROJECT_BINARY_DIR}/sac2crc.local")

# make sure that sac2c is available
# TODO: add some more paths/hints
FIND_PROGRAM (SAC2C_EXEC NAMES sac2c PATHS ${SAC2C_BUILD_DIR})
IF (NOT SAC2C_EXEC)
    MESSAGE (FATAL_ERROR "Could not located the sac2c binary, exiting...")
ENDIF ()

SET (LD_LIB_PATH "${SAC2C_BUILD_DIR}/lib:${PROJECT_BUILD_DIR}/lib:$ENV{LD_LIBRARY_PATH}")
SET (DYLD_LIB_PATH "${SAC2C_BUILD_DIR}/lib:${PROJECT_BUILD_DIR}/lib:$ENV{LD_LIBRARY_PATH}")
# FIXME: Are the headers in the correct places?
SET (SAC2C_EXTRA_INC
    -I${SAC2C_BUILD_DIR}/include
    -I${PROJECT_SOURCE_DIR}/include
    -I${PROJECT_SOURCE_DIR}/src/include
    -I${PROJECT_SOURCE_DIR}/src/libsacphm/heap
    -I${PROJECT_SOURCE_DIR}/src/libsacdistmem/commlib)

# Set environment vars for sac2c during configuration phase
# FIXME(artem) these can go directly to the call of sac2c without polluting the environment...
SET (ENV{LD_LIBRARY_PATH} ${LD_LIB_PATH})
SET (ENV{DYLD_LIBRARY_PATH} ${DYLD_LIB_PATH})
SET (ENV{SAC2CRC} "${SAC2C_BUILD_DIR}/sac2crc")

# TODO: this should be seperated
# Override existing assignments
#SET (SAC2CRC_BUILD_CONF "${SAC2C_BUILD_DIR}/sac2crc")

# added an include to sac2crc.local so that sac2c can find header files
CONFIGURE_FILE("${ISAC2CLOCAL}" "${LSAC2CLOCAL}" @ONLY)
IF (EXISTS ${LSAC2CLOCAL})
    FILE(COPY ${LSAC2CLOCAL} DESTINATION ${SAC2C_BUILD_DIR})
ELSE ()
    MESSAGE(FATAL_ERROR "Unable to copy over sac2crc.local, does not exist, exiting...")
ENDIF ()

# Make sure that all the libraries are found here.
# FIXME(artem) The name DLL_BUILD_DIR is really confusing, as sac libraries live
# in a separate project now, the DLL_BUILD_DIR of the sac2c project points into
# a different location.  Rename it?
SET (DLL_BUILD_DIR "${PROJECT_BINARY_DIR}/lib")
SET (LIBRARY_OUTPUT_PATH "${DLL_BUILD_DIR}")
# Make sure that this directory exists.
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}")
