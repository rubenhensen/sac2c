INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
SET (PRELUDE_DIR "${DLL_BUILD_DIR}/prelude")
FILE (MAKE_DIRECTORY "${PRELUDE_DIR}/${TARGET_ENV}/${SBI}")
FILE (MAKE_DIRECTORY "${PRELUDE_DIR}/tree/${TARGET_ENV}")
SET (LIBSAC_MOD "${PRELUDE_DIR}/${TARGET_ENV}/${SBI}/libsacpreludeMod${SHARED_LIB_EXT}")
SET (LIBSAC_TREE "${PRELUDE_DIR}/tree/${TARGET_ENV}/libsacpreludeTree${SHARED_LIB_EXT}")

SET (LIBSAC_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/sacprelude.sac"
)

ADD_CUSTOM_COMMAND (
    OUTPUT "${LIBSAC_MOD}" "${LIBSAC_TREE}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -linksetsize 0 -noprelude -o ${PRELUDE_DIR} ${LIBSAC_SRC}
    DEPENDS ${LIBSAC_SRC}
    COMMENT "Generating libsacprelude for target `${TARGET}'")
ADD_CUSTOM_TARGET (libsacprelude-${TARGET} ALL DEPENDS "${LIBSAC_MOD}" "${LIBSAC_TREE}")
