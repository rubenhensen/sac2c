INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsac${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

SET (LIBSAC_SRC
  cachesim/access_detailed.c
  cachesim/access_simple.c
  cachesim/basic.c
  essentials/commandline.c
  essentials/message.c
  essentials/misc.c
  essentials/profile.c
  essentials/trace.c
  interface/sacarg.c
  interface/sacargconvert.c
  interface/sacmt.c
  mt/mt.c
  mt/hwloc_data.c
  mt_beehive/mt_beehive.c
  mt_lpel/mt_lpel_std.c
  mt_lpel/mt_lpel_trace.c
  mt_omp/mt_omp.c
  mt_pth/mt_pth_std.c
  mt_pth/mt_pth_trace.c
  mt_smart/mt_smart.c
)

IF(${RTSPEC} EQUAL 1)
  MESSAGE ("Target `${TARGET}' does support RTSPEC")
ELSE ()
  MESSAGE ("Target `${TARGET}' does *not* support RTSPEC")
  LIST (APPEND LIBSAC_SRC rtspec/empty.c)
ENDIF ()

SET(LIBSAC_OBJS)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.libsac${OBJEXT}")
  ADD_CUSTOM_COMMAND (
    OUTPUT "${dst}"
    DEPENDS "${src}" ${LIB_SAC2C}
    COMMAND ${SAC2C} -v0 -noprelude -cc ccmod -o "${dst}" "${src}"
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND LIBSAC_OBJS "${dst}")
ENDFOREACH(name)

ADD_CUSTOM_COMMAND (
  OUTPUT "${LIBSAC_LIB}"
  COMMAND  ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT}
           -o "${LIBSAC_LIB}" ${LIBSAC_OBJS}
  DEPENDS ${LIBSAC_OBJS}
  COMMENT "Linking for target `${TARGET}' libsac")
ADD_CUSTOM_TARGET (libsac-${TARGET} ALL DEPENDS "${LIBSAC_LIB}")
