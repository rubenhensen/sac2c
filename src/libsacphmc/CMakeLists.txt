INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacphmc${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")
SET (LIBSAC_LIBD "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacphmc.diag${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

SET (LIBSAC_SRC wrappers/nophm.c)

SET(LIBSAC_OBJS)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.phmc${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
    COMMAND ${SAC2C} -v0 -noprelude -cc ccmod -o "${dst}" "${src}"
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND LIBSAC_OBJS "${dst}")
ENDFOREACH(name)

SET(LIBSAC_OBJSD)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.phmc.diag${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
  COMMAND ${SAC2C} -v0 -noprelude -cc ccmod -Xc -DDIAG -o "${dst}" "${src}"
  COMMENT "Generating ${dst} (diag)"
  )
  LIST (APPEND LIBSAC_OBJSD "${dst}")
ENDFOREACH(name)

ADD_CUSTOM_COMMAND (
  OUTPUT "${LIBSAC_LIB}"
  COMMAND ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIB}" ${LIBSAC_OBJS}
  DEPENDS ${LIBSAC_OBJS} #libsacphmc-${TARGET}-objs
  COMMENT "Linking for target `${TARGET}' libsacphmc")

ADD_CUSTOM_COMMAND (
  OUTPUT "${LIBSAC_LIBD}"
  COMMAND ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIBD}" ${LIBSAC_OBJSD}
  DEPENDS ${LIBSAC_OBJSD} #libsacphmc-${TARGET}-objsd
  COMMENT "Linking for target `${TARGET}' libsacphmc (diag)")

# Create and link both the normal and DIAG version of libsacphm
ADD_CUSTOM_TARGET (libsacphmc-${TARGET} ALL DEPENDS "${LIBSAC_LIB}" "${LIBSAC_LIBD}")
