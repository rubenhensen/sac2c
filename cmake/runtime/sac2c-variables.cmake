# Create local variant of the SAC2C flags
SET (SAC2C_T ${SAC2C_EXEC} -target ${TARGET})

# In case of BUILDGENERIC we need to propagate the -generic flag
# to all the sac libraries.
IF (BUILDGENERIC)
    SET (SAC2C_T ${SAC2C_T} -generic)
ENDIF ()

SET (SAC2C ${SAC2C_T} -Xp "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"")
SET (SAC2C_NT ${SAC2C_EXEC} -Xp "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"") # defaults to SEQ

# set environment vars - these are only used during configuration and are lost there after
# we them because EXECUTE_PROCESS does not support passing in vars via the command.
SET (ENV{LD_LIBRARY_PATH} ${LD_LIB_PATH})
SET (ENV{DYLD_LIBRARY_PATH} ${DYLD_LIB_PATH})
SET (ENV{SAC2CRC} "${SAC2C_BUILD_DIR}/sac2crc")

# get the target environment - possibly `x64' or similar...
EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CTARGET_ENV OUTPUT_VARIABLE TARGET_ENV OUTPUT_STRIP_TRAILING_WHITESPACE)

# do several tests to make sure that SBI data is sane...
IF (NOT TARGET_ENV)
    MESSAGE (FATAL_ERROR "${SAC2C_T} seems not to work, cannot determine SBI data, exiting...")
ELSE ()
    MACRO (GET_SAC2C_VAR var)
    	EXECUTE_PROCESS (COMMAND ${SAC2C_T} -C${var} OUTPUT_VARIABLE  ${var}  OUTPUT_STRIP_TRAILING_WHITESPACE)
    ENDMACRO ()

    GET_SAC2C_VAR (SBI)
    GET_SAC2C_VAR (OBJEXT)
    GET_SAC2C_VAR (MODEXT)
    GET_SAC2C_VAR (TREE_DLLEXT)
    GET_SAC2C_VAR (USE_PHM_API)
    GET_SAC2C_VAR (RTSPEC)

    # FIXME(artem) Isn't SHARED_LIB_EXT and MODEXT the same thing?
    # XXX(hans): yes, but not in the case when the target environment is set by the sac2crc file...
ENDIF ()

IF ("${SBI}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No SBI specification for target `${TARGET}', exiting...")
ENDIF ()
IF ("${TARGET_ENV}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No TARGET_ENV specificed for target `${TARGET}', exiting...")
ENDIF ()

MESSAGE (STATUS "Target `${TARGET}' build properties: ${TARGET_ENV} ${SBI} ${SHARED_LIB_EXT} ${OBJEXT} ${TREEEXT} ${USE_PHM_API} ${RTSPEC}")
