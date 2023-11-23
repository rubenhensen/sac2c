HEADER_DIRECTORIES (DIR_LIST)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")

IF(${USE_PHM_API} EQUAL 1)
  MESSAGE ("Target `${TARGET}' does support PHM")
  SET (LIBSAC_SRC
       compat/malloc.c
       diag/diagnostics.c
       heap/internal.c
       heap/large_chunks.c
       heap/setup.c
       heap/small_chunks.c
       heap/thread_ids.c
       heap/wrapper.c
  )
ELSE ()
  MESSAGE ("Target `${TARGET}' does *not* support PHM")
  SET (LIBSAC_SRC empty/empty.c)
ENDIF ()

INCLUDE_DIRECTORIES (${DIR_LIST})
ADD_LIBRARY (libsacphm-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacphm-${TARGET}
  PROPERTIES
    OUTPUT_NAME sacphm
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)

ADD_LIBRARY (libsacphmdiag-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacphmdiag-${TARGET}
  PROPERTIES
    COMPILE_DEFINITIONS DIAG
    OUTPUT_NAME sacphm.diag
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)
