HEADER_DIRECTORIES (DIR_LIST)
INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}")

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

INCLUDE_DIRECTORIES (${DIR_LIST})
ADD_LIBRARY (libsacdistmem-${TARGET} SHARED ${LIBSAC_SRC})
SET_TARGET_PROPERTIES (libsacdistmem-${TARGET}
  PROPERTIES
    OUTPUT_NAME sacdistmem
    LIBRARY_OUTPUT_DIRECTORY  "${DLL_BUILD_DIR}/${TARGET_ENV}/${SBI}"
    # we *must* not put any fpic in here as some compilers (eg nvcc) cannot handle this
    # this implies that the COMPILE_RMOD setup needs to include this vi CFLAGS!
    POSITION_INDEPENDENT_CODE False
)
