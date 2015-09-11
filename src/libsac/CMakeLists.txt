# A Pattern to build a variant of the libsac.
# When including this CMakeLists.txt as subdirectory
# one shoude define variables:
#
#       * CC_FLAGS_MT            comes from cmake/settings.cmake.
#
#       * CC_FLAGS_MT_VARIANT    extra definitions for the variant of the
#                                libsacphm library.
#
#       * MT_VARIANT_NAME        the name of the resulting variable without
#                                the postfix.
#
# For example, the following inclusion is valid:
#
#       SET (CC_FLAGS_MT_VARIANT        -DMT -DPTH)
#       SET (MT_VARIANT_NAME            libsac.mt)
#       ADD_SUBDIRECTORY (libsacphm     libsac.mt)
#


# FIXME this is WRONG, this condition should not check for ENABLE_MUTC
# but rather for CROSS==MUTC.
IF (NOT ENABLE_MUTC)
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
    mt_beehive/mt_beehive.c
    mt_lpel/mt_lpel_std.c
    mt_lpel/mt_lpel_trace.c
    mt_omp/mt_omp.c
    mt_pth/mt_pth_std.c
    mt_pth/mt_pth_trace.c
    phm/nophm.c
    rtspec/controller.c
    rtspec/persistence.c
    rtspec/registry.c
    rtspec/reqqueue.c
  )
ELSE ()
  SET (LIBSAC_SRC
    essentials/commandline.c
    essentials/message.c
    essentials/misc.c
    essentials/profile.c
    essentials/trace.c
    interface/sacarg.c
    interface/sacargconvert.c
    interface/sacmt.c
    mutc/reference_counting.c
    phm/nophm.c
  )
ENDIF (NOT ENABLE_MUTC)

ADD_DEFINITIONS(
    ${CC_FLAGS_MT}
    ${CC_FLAGS_MT_VARIANT}
)

# FIXME This probably can be changed later.
SET (LIB_PATH "${PROJECT_SOURCE_DIR}/lib")

SET (SHARED_TARGET_NAME    "${MT_VARIANT_NAME}.Shared")
SET (STATIC_TARGET_NAME    "${MT_VARIANT_NAME}.Static")

ADD_LIBRARY (${SHARED_TARGET_NAME} SHARED ${LIBSAC_SRC})
ADD_LIBRARY (${STATIC_TARGET_NAME} STATIC ${LIBSAC_SRC})
ADD_DEPENDENCIES (${SHARED_TARGET_NAME} sac_h)
ADD_DEPENDENCIES (${STATIC_TARGET_NAME} sac_h)

SET_TARGET_PROPERTIES (${SHARED_TARGET_NAME}
    PROPERTIES
        OUTPUT_NAME ${MT_VARIANT_NAME}
        LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH}
)
SET_TARGET_PROPERTIES (${STATIC_TARGET_NAME}
    PROPERTIES
        OUTPUT_NAME ${MT_VARIANT_NAME}
        ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH}
)

