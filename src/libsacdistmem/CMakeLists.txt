INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")
SET (LIBSAC_LIB "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}/libsacdistmem${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}")

SET (LIBSAC_SRC
    distmem/distmem.c
    distmem/distmem_trace.c
    distmem/distmem_profile.c
    commlib/distmem_commlib_gasnet.c
    commlib/distmem_commlib_gasnet_trace.c
    commlib/distmem_commlib_mpi.c
    commlib/distmem_commlib_mpi_trace.c
    commlib/distmem_commlib_armci.c
    commlib/distmem_commlib_armci_trace.c
    commlib/distmem_commlib_gpi.c
    commlib/distmem_commlib_gpi_trace.c
)

SET(LIBSAC_OBJS)
FOREACH(name ${LIBSAC_SRC})
  SET (src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  GET_FILENAME_COMPONENT (dst ${name} NAME_WE)
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${dst}.libsacdistmem${OBJEXT}")
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
  COMMAND ${SAC2C} -v0 -linksetsize 0 -noprelude -cc ldmod ${LIB_M} ${LIB_RT} -o "${LIBSAC_LIB}" ${LIBSAC_OBJS} 
  DEPENDS ${LIBSAC_OBJS}
  COMMENT "Linking for target `${TARGET}' libsac")
ADD_CUSTOM_TARGET (libsacdistmem-${TARGET} ALL DEPENDS "${LIBSAC_LIB}")
