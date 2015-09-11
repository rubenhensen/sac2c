ADD_SUBDIRECTORY (libsac2c)
ADD_SUBDIRECTORY (runtime)

# Include the libsac or libsacphm subdirectory and pre-set the variables
# of the variant that we want to compile.
MACRO (ADD_LIBSAC_TARGET SRC_DIR VARIANT_NAME VARIANT_FLAGS)
    SET (CC_FLAGS_MT_VARIANT        ${VARIANT_FLAGS})
    SET (MT_VARIANT_NAME            ${VARIANT_NAME})
    ADD_SUBDIRECTORY (${SRC_DIR}    ${VARIANT_NAME})
ENDMACRO ()

# Generate variants of the libsacphm
# FIXME check for CROSS_BACKEND == MUTC is missing
# FIXME CROSS variant is missing
IF (ENABLE_PHM)
    ADD_LIBSAC_TARGET (libsacphm  sacphm.seq       "${DEAULT_CFLAGS}")
    ADD_LIBSAC_TARGET (libsacphm  sacphm.seq.diag  "${DEAULT_CFLAGS} ${SETTINGS_DIAG}")
    ADD_LIBSAC_TARGET (libsacphm  sacphm.mt        "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_PTH}")
    ADD_LIBSAC_TARGET (libsacphm  sacphm.mt.diag   "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_PTH} ${SETTINGS_DIAG}")
    ADD_LIBSAC_TARGET (libsacphm  sacphm.xt        "${CFLAGS_MT} ${SETTINGS_XT} ${SETTINGS_PTH}")
    ADD_LIBSAC_TARGET (libsacphm  sacphm.xt.diag   "${CFLAGS_MT} ${SETTINGS_XT} ${SETTINGS_PTH} ${SETTINGS_DIAG}")

ENDIF ()


ADD_LIBSAC_TARGET (libsac  sac.seq       "${DEAULT_CFLAGS}")
ADD_LIBSAC_TARGET (libsac  sac.mt        "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_PTH}")
ADD_LIBSAC_TARGET (libsac  sac.mt.omp    "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_OMP}")
ADD_LIBSAC_TARGET (libsac  sac.xt        "${CFLAGS_MT} ${SETTINGS_XT} ${SETTINGS_PTH}")
IF (NOT CYGWIN)
    ADD_LIBSAC_TARGET (libsac  sac.mt.lpel   "${CFLAGS_MT} ${SETTINGS_MT} ${SETTINGS_LPEL}")
ENDIF ()

# FIXME add distmem sutff.
