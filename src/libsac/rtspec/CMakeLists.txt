HEADER_DIRECTORIES (DIR_LIST)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacrtspec${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

IF(${RTSPEC} EQUAL 1)
  MESSAGE ("Target `${TARGET}' does support RTSPEC")
  SET (LIBSAC_SRC
       persistence.c
       registry.c
       simple_controller.c
       simple_reqqueue.c
       supervisor.c
       uuid_controller.c
       uuid_reqqueue.c
  )
ELSE ()
  MESSAGE ("Target `${TARGET}' does *not* support RTSPEC")
  SET (LIBSAC_SRC ../../libsacphm/empty/empty.c)
ENDIF ()

INCLUDE_DIRECTORIES (${DIR_LIST})
ADD_LIBRARY (libsacrtspec-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacrtspec-${TARGET}
  PROPERTIES
    OUTPUT_NAME sacrtspec
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)
