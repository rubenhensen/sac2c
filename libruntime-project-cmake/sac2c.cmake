# file include checks to find sac2c, etc.

SET (SAC2CLOCAL "${PROJECT_BINARY_DIR}/sac2crc.local")
SET (BSAC2CLOCAL "${SAC2C_BUILD_DIR}/sac2crc.local")

# make sure that sac2c is available
# TODO: add some more paths/hints
FIND_PROGRAM (SAC2C_EXEC NAMES sac2c PATHS ${SAC2C_BUILD_DIR})
IF (NOT SAC2C_EXEC)
    MESSAGE (FATAL_ERROR "Could not located the sac2c binary, exiting...")
ENDIF ()

SET (LD_LIB_PATH "${SAC2C_BUILD_DIR}/lib:${DLL_BUILD_DIR}:$ENV{LD_LIBRARY_PATH}")
SET (DYLD_LIB_PATH "${SAC2C_BUILD_DIR}/lib:${DLL_BUILD_DIR}:$ENV{LD_LIBRARY_PATH}")
# FIXME: Are the headers in the correct places?
SET (SAC2C_EXTRA_INC
    -I${SAC2C_BUILD_DIR}/include
    -I${PROJECT_SOURCE_DIR}/include
    -I${PROJECT_SOURCE_DIR}/src/include
    -I${PROJECT_SOURCE_DIR}/src/libsacphm/heap
    -I${PROJECT_SOURCE_DIR}/src/libsacdistmem/commlib)

# Set environment vars for sac2c during configuration phase
SET (ENV{LD_LIBRARY_PATH} ${LD_LIB_PATH})
SET (ENV{DYLD_LIBRARY_PATH} ${DYLD_LIB_PATH})
SET (ENV{SAC2CRC} ${SAC2CRC_BUILD_CONF})

# added an include to sac2crc.local so that sac2c can find header files
IF (EXISTS ${PROJECT_BINARY_DIR}/sac2crc.local)
    FILE (READ "${SAC2CLOCAL}" SAC2CLOCAL)
    STRING (REGEX REPLACE "SACINCLUDES[ ]*\\+=[ ]*\"([^\"]+)\"" "SACINCLUDES    += \"\\1 -I${SAC2C_BUILD_DIR}/include\"" NSAC2CLOCAL "${SAC2CLOCAL}")
    FILE (WRITE "${BSAC2CLOCAL}" "${NSAC2CLOCAL}")
ELSE ()
    MESSAGE(FATAL_ERROR "Unable to copy over sac2crc.local, does not exist, exiting...")
ENDIF ()
