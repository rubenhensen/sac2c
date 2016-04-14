CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Name of the project
PROJECT (libsac C)

IF (NOT SAC2C_BUILD_DIR)
    MESSAGE (FATAL_ERROR "The SAC2C_BUILD_DIR is not set!")
ENDIF ()
IF (NOT EXISTS "${SAC2C_BUILD_DIR}/sac2crc.local")
    MESSAGE (FATAL_ERROR
        "The content of SAC2C_BUILD_DIR does not contain sac2crc.local.\
        Please ensure that you have compiled the sac2c project and that\
        sac2crc.local has been generated.")
ENDIF ()

GET_FILENAME_COMPONENT (SAC2C_BUILD_DIR "${SAC2C_BUILD_DIR}" ABSOLUTE)

# we include the global config files
include ("cmake/macros.cmake")
include ("cmake/options.cmake")
include ("cmake/config.cmake")
include ("cmake/settings.cmake")

# and include our for checks on sac2c
include ("sac2c.cmake")

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "${SAC2C_BUILD_DIR}/include"
  "${SAC2C_BUILD_DIR}/src/include"
  "${SAC2C_BUILD_DIR}/src/libsacphm/heap"
  "${SAC2C_BUILD_DIR}/src/libsacdistmem/commlib"
  "${PROJECT_BINARY_DIR}/include"
)

# Include the libsac or libsacphm subdirectory and pre-set the variables
# of the variant that we want to compile.
MACRO (ADD_LIBSAC_TARGET SRC_DIR VARIANT_TARGET)
    SET (TARGET                     ${VARIANT_TARGET})
    ADD_SUBDIRECTORY (${SRC_DIR}    ${SRC_DIR}-${TARGET})
ENDMACRO ()

# TODO: probably some more...
FOREACH (TARGET IN ITEMS ${TARGETS})
    ADD_LIBSAC_TARGET (src/libsac ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphm ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphmc ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacdistmem ${TARGET})
    ADD_LIBSAC_TARGET (src/libsac/rtspec ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacprelude ${TARGET})
ENDFOREACH ()

# for tools
ADD_SUBDIRECTORY (src/tools)
