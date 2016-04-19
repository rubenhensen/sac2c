# Create local variant of the SAC2C flags
SET (SAC2C_T ${SAC2C_EXEC} -target ${TARGET})
SET (SAC2C ${SAC2C_T} -Xc "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"")
SET (SAC2C_NT ${SAC2C} -Xc "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"") # defaults to SEQ

EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CTARGET_ENV OUTPUT_VARIABLE TARGET_ENV OUTPUT_STRIP_TRAILING_WHITESPACE)
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
ENDIF ()

IF ("${SBI}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No SBI specification for target `${TARGET}', exiting...")
ENDIF ()
IF ("${TARGET_ENV}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No TARGET_ENV specificed for target `${TARGET}', exiting...")
ENDIF ()

MESSAGE (STATUS "Target `${TARGET}' build properties: ${TARGET_ENV} ${SBI} ${SHARED_LIB_EXT} ${OBJEXT} ${TREEEXT} ${USE_PHM_API} ${RTSPEC}")
