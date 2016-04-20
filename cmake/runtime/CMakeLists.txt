CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (sac2c_runtime C)

IF (NOT SAC2C_BUILD_DIR)
    MESSAGE (FATAL_ERROR "The SAC2C_BUILD_DIR is not set!")
ENDIF ()
IF (NOT SAC2C_SOURCE_DIR)
    MESSAGE (FATAL_ERROR "The SAC2C_SOURCE_DIR is not set!")
ENDIF ()

IF (NOT EXISTS "${SAC2C_BUILD_DIR}/sac2crc.local")
    MESSAGE (FATAL_ERROR
        "The content of SAC2C_BUILD_DIR does not contain sac2crc.local.\
        Please ensure that you have compiled the sac2c project and that\
        sac2crc.local has been generated.")
ENDIF ()
#XXX: maybe we should check for something else, like cmake/ dir?
IF (NOT EXISTS "${SAC2C_SOURCE_DIR}/src/libsac/essentials/commandline.c")
    MESSAGE (FATAL_ERROR
        "The content of SAC2C_SOURCE_DIR does not contain the src/libsac/\
        directory structure. Please specify this location.")
ENDIF ()

GET_FILENAME_COMPONENT (SAC2C_BUILD_DIR "${SAC2C_BUILD_DIR}" ABSOLUTE)
GET_FILENAME_COMPONENT (SAC2C_SOURCE_DIR "${SAC2C_SOURCE_DIR}" ABSOLUTE)

# we include the global config files
include ("${SAC2C_SOURCE_DIR}/cmake/macros.cmake")
include ("${SAC2C_SOURCE_DIR}/cmake/options.cmake")
#include ("${SAC2C_SOURCE_DIR}/cmake/config.cmake")
include ("${SAC2C_SOURCE_DIR}/cmake/settings.cmake")

# and include our for checks on sac2c
include ("${SAC2C_SOURCE_DIR}/cmake/runtime/config.cmake")

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "${SAC2C_BUILD_DIR}/include"
  "${SAC2C_BUILD_DIR}/src/include"
  "${SAC2C_BUILD_DIR}/src/libsacphm/heap"
  "${SAC2C_BUILD_DIR}/src/libsacdistmem/commlib"
  "${PROJECT_BINARY_DIR}/include"
)

# Include the runtime subdirectory and pre-set the variables
# of the variant that we want to compile.
MACRO (ADD_LIBSAC_TARGET SRC_DIR VARIANT_TARGET)
    SET (TARGET                     ${VARIANT_TARGET})
    # FIXME: this isn't very `clean'... 
    ADD_SUBDIRECTORY (${SAC2C_SOURCE_DIR}/${SRC_DIR}    ${SRC_DIR}-${TARGET})
ENDMACRO ()

FOREACH (TARGET IN ITEMS ${TARGETS})
    ADD_LIBSAC_TARGET (src/libsac ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphm ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphmc ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacdistmem ${TARGET})
    ADD_LIBSAC_TARGET (src/libsac/rtspec ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacprelude ${TARGET})
ENDFOREACH ()

# for tools
ADD_LIBSAC_TARGET (src/tools seq)
