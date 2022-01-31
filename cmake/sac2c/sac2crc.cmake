# These are generated at 'configure' time and are only
# necessary for the sac2c build.

# sac2crc and makefile-related variables
MACRO (SUBST_SAC2CRC_FILE f var)
    CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/setup/${f}.in" "${PROJECT_BINARY_DIR}/${f}")
    FILE (READ "${PROJECT_BINARY_DIR}/${f}" ${var})
ENDMACRO ()

SUBST_SAC2CRC_FILE ("sac2crc.backend.mutc" RCMUTC)
IF (ENABLE_CUDA)
    SUBST_SAC2CRC_FILE ("sac2crc.backend.cuda" RCCUDA)
ENDIF ()
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.cc" RCCC)
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.malloc" RCMALLOC)
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.rcm" RCRCM)
SUBST_SAC2CRC_FILE ("sac2crc.SUN" RCSUN)
SUBST_SAC2CRC_FILE ("sac2crc.X86" RCX86)
SUBST_SAC2CRC_FILE ("sac2crc.ALPHA" RCALPHA)
SUBST_SAC2CRC_FILE ("sac2crc.MAC" RCMAC)

CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/setup/sac2crc.pre.in" "${PROJECT_BINARY_DIR}/sac2crc.pre" @ONLY)

SET (abs_top_srcdir "${PROJECT_BINARY_DIR}")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/cmake/sac2c/sac2crc.local.in" "${PROJECT_BINARY_DIR}/sac2crc${BUILD_TYPE_POSTFIX}.local" @ONLY)
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/cmake/sac2c/sac2crc.prelude.in" "${PROJECT_BINARY_DIR}/sac2crc.${BUILD_TYPE_NAME}.prelude" @ONLY)
CONFIGURE_FILE ("${PROJECT_BINARY_DIR}/sac2crc.pre" "${SAC2CRC_BUILD_CONF}")
