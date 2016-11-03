INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacrtspec${SHARED_LIB_EXT}")

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

SET(LIBSAC_OBJS)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.rtspec${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -noprelude -cc ccmod -o "${dst}" "${src}"
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND LIBSAC_OBJS "${dst}")
ENDFOREACH(name)
#ADD_CUSTOM_TARGET (libsacrtspec-${TARGET}-objs DEPENDS ${LIBSAC_OBJS})

ADD_CUSTOM_COMMAND (
    OUTPUT "${LIBSAC_LIB}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIB}" ${LIBSAC_OBJS}
    DEPENDS ${LIBSAC_OBJS} #libsacrtspec-${TARGET}-objs
    COMMENT "Linking for target `${TARGET}' libsacrtspec")
ADD_CUSTOM_TARGET (libsacrtspec-${TARGET} ALL DEPENDS "${LIBSAC_LIB}")
