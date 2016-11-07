INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacphm${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")
SET (LIBSAC_LIBD "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacphm.diag${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

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

SET(LIBSAC_OBJS)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -noprelude -cc ccmod -o "${dst}" "${src}"
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND LIBSAC_OBJS "${dst}")
ENDFOREACH(name)
#ADD_CUSTOM_TARGET (libsacphm-${TARGET}-objs DEPENDS ${LIBSAC_OBJS})

SET(LIBSAC_OBJSD)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.diag${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -noprelude -cc ccmod -Xc -DDIAG -o "${dst}" "${src}"
COMMENT "Generating ${dst} (diag)"
  )
  LIST (APPEND LIBSAC_OBJSD "${dst}")
ENDFOREACH(name)
#ADD_CUSTOM_TARGET (libsacphm-${TARGET}-objsd DEPENDS ${LIBSAC_OBJSD})

ADD_CUSTOM_COMMAND (
    OUTPUT "${LIBSAC_LIB}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIB}" ${LIBSAC_OBJS}
    DEPENDS ${LIBSAC_OBJS} #libsacphm-${TARGET}-objs
    COMMENT "Linking for target `${TARGET}' libsacphm")

ADD_CUSTOM_COMMAND (
    OUTPUT "${LIBSAC_LIBD}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIBD}" ${LIBSAC_OBJSD}
DEPENDS ${LIBSAC_OBJSD} #libsacphm-${TARGET}-objsd
    COMMENT "Linking for target `${TARGET}' libsacphm (diag)")

# Create and link both the normal and DIAG version of libsacphm
ADD_CUSTOM_TARGET (libsacphm-${TARGET} ALL DEPENDS "${LIBSAC_LIB}" "${LIBSAC_LIBD}")
