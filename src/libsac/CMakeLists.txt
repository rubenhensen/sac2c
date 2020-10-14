HEADER_DIRECTORIES (DIR_LIST)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")


# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
#SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsac${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

SET (LIBSAC_SRC
  cachesim/access_detailed.c
  cachesim/access_simple.c
  cachesim/basic.c
  essentials/commandline.c
  essentials/message.c
  essentials/misc.c
  essentials/trace.c
  profile/profile_print.c
  profile/profile_memory.c
  profile/profile_ops.c
  interface/sacarg.c
  interface/sacargconvert.c
  interface/sacmt.c
  hwloc/cudabind.c
  hwloc/cpubind.c
  mt/mt.c
  mt/hwloc_data.c
  mt/mt_beehive.c
  mt/mt_lpel.c
  mt/mt_omp.c
  mt/mt_pth.c
  mt/mt_smart.c
  mt/mt_barriers.c
)

IF(${RTSPEC} EQUAL 1)
  MESSAGE ("Target `${TARGET}' does support RTSPEC")
ELSE ()
  MESSAGE ("Target `${TARGET}' does *not* support RTSPEC")
  LIST (APPEND LIBSAC_SRC rtspec/empty.c)
ENDIF ()


INCLUDE_DIRECTORIES (${DIR_LIST})
ADD_LIBRARY (libsac-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsac-${TARGET}
  PROPERTIES
    OUTPUT_NAME sac
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
)
