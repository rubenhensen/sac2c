# Create local variant of the SAC2C flags 
SET (SAC2C_T ${SAC2C_EXEC} -target ${TARGET})
SET (SAC2C ${SAC2C_T} -Xc "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"")
SET (SAC2C_NT ${SAC2C} -Xc "\"${SAC2C_EXTRA_INC}\"" -Xtc "\"${SAC2C_EXTRA_INC}\"") # defaults to SEQ

EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CTARGET_ENV OUTPUT_VARIABLE TARGET_ENV OUTPUT_STRIP_TRAILING_WHITESPACE)
IF (NOT TARGET_ENV)
    MESSAGE (FATAL_ERROR "${SAC2C_T} seems not to work, cannot determine SBI data, exiting...")
ELSE ()
    EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CSBI OUTPUT_VARIABLE SBI OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND ${SAC2C_T} -COBJEXT OUTPUT_VARIABLE OBJEXT OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CTREE_DLLEXT OUTPUT_VARIABLE TREEEXT OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CUSE_PHM_API OUTPUT_VARIABLE USE_PHM_API OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND ${SAC2C_T} -CRTSPEC OUTPUT_VARIABLE RTSPEC OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF ()

IF ("${SBI}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No SBI specification for target `${TARGET}', exiting...")
ENDIF ()
IF ("${TARGET_ENV}" STREQUAL "XXXXX")
    MESSAGE (FATAL_ERROR "No TARGET_ENV specificed for target `${TARGET}', exiting...")
ENDIF ()

MESSAGE (STATUS "Target `${TARGET}' build properties: ${TARGET_ENV} ${SBI} ${SHARED_LIB_EXT} ${OBJEXT} ${TREEEXT} ${USE_PHM_API} ${RTSPEC}")

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
    DEPENDS "${src}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -noprelude -cc ccmod -o "${dst}" "${src}"
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND LIBSAC_OBJS "${dst}")
ENDFOREACH(name)
ADD_CUSTOM_TARGET (libsacrtspec-${TARGET}-objs DEPENDS ${LIBSAC_OBJS})

ADD_CUSTOM_COMMAND (
    OUTPUT "${LIBSAC_LIB}"
# FIXME: we currently pass some environment variables as is... is this compatible on other platforms?
    COMMAND SAC2CRC=${SAC2CRC_BUILD_CONF} LD_LIBRARY_PATH=${LD_LIB_PATH} DYLD_LIBRARY_PATH=${DYLD_LIB_PATH} ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIB}" ${LIBSAC_OBJS} 
    DEPENDS libsacrtspec-${TARGET}-objs
    COMMENT "Linking for target `${TARGET}' libsacrtspec")
ADD_CUSTOM_TARGET (libsacrtspec-${TARGET} ALL DEPENDS "${LIBSAC_LIB}")
