# A Pattern to build a variant of the libsacphm.
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
#       SET (MT_VARIANT_NAME            libsacphm.mt)
#       ADD_SUBDIRECTORY (libsacphm     libsacphm.mt)
#

# Get a list of subdirectories containing *.h files into the variable HEADER_LIST
HEADER_DIRECTORIES (HEADER_LIST)

# Add each directory to include_dirs.
FOREACH (d ${HEADER_LIST})
  INCLUDE_DIRECTORIES ("${d}")
ENDFOREACH (d)


SET (PHM_SRC
    heap/internal.c
    heap/setup.c
    heap/wrapper.c
    heap/small_chunks.c
    heap/large_chunks.c
    heap/thread_ids.c
    diag/diagnostics.c
    opt/placement.c
    compat/malloc.c
)

ADD_DEFINITIONS(
    ${CC_FLAGS_MT}
    ${CC_FLAGS_MT_VARIANT}
)

# FIXME This probably can be changed later.
SET (LIB_PATH "${PROJECT_SOURCE_DIR}/lib")

SET (SHARED_TARGET_NAME    "${MT_VARIANT_NAME}.Shared")
SET (STATIC_TARGET_NAME    "${MT_VARIANT_NAME}.Static")


ADD_LIBRARY (${SHARED_TARGET_NAME} SHARED ${PHM_SRC})
ADD_LIBRARY (${STATIC_TARGET_NAME} STATIC ${PHM_SRC})
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

