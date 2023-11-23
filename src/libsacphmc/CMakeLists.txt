HEADER_DIRECTORIES (DIR_LIST)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")

SET (LIBSAC_SRC wrappers/nophm.c)

INCLUDE_DIRECTORIES (${DIR_LIST})
ADD_LIBRARY (libsacphmc-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacphmc-${TARGET}
  PROPERTIES
    OUTPUT_NAME sacphmc
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)

ADD_LIBRARY (libsacphmcdiag-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacphmcdiag-${TARGET}
  PROPERTIES
    COMPILE_DEFINITIONS DIAG
    OUTPUT_NAME sacphmc.diag
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)
