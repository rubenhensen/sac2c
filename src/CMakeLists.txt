SET (CFLAGS "-Wall -g -std=c99")
ADD_DEFINITIONS (${CFLAGS})

ADD_SUBDIRECTORY (libsac2c)
ADD_SUBDIRECTORY (runtime)
ADD_SUBDIRECTORY (libsac)

# Include the libsacphm subdirectory and pre-set the variables of the phm
# variant that we want to compile.
MACRO (ADD_PHM_TARGET VARIANT_NAME VARIANT_FLAGS)
    SET (CC_FLAGS_MT_VARIANT        ${VARIANT_FLAGS})
    SET (MT_VARIANT_NAME            ${VARIANT_NAME})
    ADD_SUBDIRECTORY (libsacphm     ${VARIANT_NAME})
ENDMACRO ()

# FIXME check that this is not the MUTC backend.
IF (ENABLE_PHM)
    ADD_PHM_TARGET (libsacphm.seq       "${DEAULT_CFLAGS}")
    ADD_PHM_TARGET (libsacphm.seq.diag  "${DEAULT_CFLAGS} ${SETTINGS_DIAG}")
    ADD_PHM_TARGET (libsacphm.mt        "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_PTH}")
    ADD_PHM_TARGET (libsacphm.mt.diag   "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_PTH} ${SETTINGS_DIAG}")
    ADD_PHM_TARGET (libsacphm.xt        "${CFLAGS_MT} ${SETTINGS_XT} ${SETTINGS_PTH}")
    ADD_PHM_TARGET (libsacphm.xt.diag   "${CFLAGS_MT} ${SETTINGS_XT} ${SETTINGS_PTH} ${SETTINGS_DIAG}")

    # FIXME the cross variant is still missing.
ENDIF ()
