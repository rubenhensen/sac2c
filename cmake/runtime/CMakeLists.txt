CMAKE_MINIMUM_REQUIRED (VERSION 3.17)

# Name of the project
PROJECT (sac2c_runtime C)



IF (NOT SAC2C_BUILD_DIR)
    MESSAGE (FATAL_ERROR "The SAC2C_BUILD_DIR is not set!")
ENDIF ()
IF (NOT SAC2C_SOURCE_DIR)
    MESSAGE (FATAL_ERROR "The SAC2C_SOURCE_DIR is not set!")
ENDIF ()
IF (NOT RT_TARGETS)
    MESSAGE (FATAL_ERROR "Build targets are missing!")
ENDIF ()

SET (CMAKE_${CMAKE_BUILD_TYPE}_POSTFIX "${BUILD_TYPE_POSTFIX}")

IF (NOT EXISTS "${SAC2C_BUILD_DIR}/sac2crc${BUILD_TYPE_POSTFIX}.local")
    MESSAGE (FATAL_ERROR
        "The content of SAC2C_BUILD_DIR does not contain sac2crc${BUILD_TYPE_POSTFIX}.local.\
        Please ensure that you have compiled the sac2c project and that\
        sac2crc${BUILD_TYPE_POSTFIX}.local has been generated.")
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

# and include our for checks on sac2c
include ("${SAC2C_SOURCE_DIR}/cmake/runtime/config.cmake")

# Include common directories for the project.
INCLUDE_DIRECTORIES(
  "${SAC2C_BUILD_DIR}/include"
  "${SAC2C_SOURCE_DIR}/src/include"
  "${SAC2C_BUILD_DIR}/src/libsacphm/heap"
  "${SAC2C_BUILD_DIR}/src/libsacdistmem/commlib"
  "${PROJECT_BINARY_DIR}/include"
)

# This is a path to libsac2c library. Make sure that
# ALL the subsequent targets in the runtime library
# add a dependency to this variable.  Otherwised they
# won't get rebuild when libsac2c is modified.
FIND_LIBRARY (LIB_SAC2C NAMES "libsac2c${BUILD_TYPE_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
              PATHS ${SAC2C_BUILD_DIR}/lib NO_DEFAULT_PATH)

# Include the runtime subdirectory and pre-set the variables
# of the variant that we want to compile.
MACRO (ADD_LIBSAC_TARGET SRC_DIR VARIANT_TARGET)
    SET (TARGET                     ${VARIANT_TARGET})

    # We overload recepies on how to build .c -> .o and *.o -> .so
    # First we set the actual C compile to the script that will print
    # an error message and die.
    SET (CMAKE_C_COMPILER "${SAC2C_BUILD_DIR}/fake-cc")

    # Further, we overload the build-recepies for building object files,
    # libraries and executables.  Hopefully this is enough to build everything
    # we need.
    # NOTE (hans) we use the `rmod` target which uses compiler and linker specified
    # through the RCC and RLD sac2crc variables (additionally we use
    SET (SACCC "${SAC2C_BUILD_DIR}/saccc ${SAC2C_EXEC} rmod ${TARGET}")

    # In case we are using BUILDGENERIC, we need propogate the sac2c `-generic`
    # flag to all the C files that we compile.
    IF (BUILDGENERIC)
        SET (SACCC "${SACCC} -Xs '-generic'")
    ENDIF ()

    SET (CMAKE_C_COMPILE_OBJECT
        "${SACCC} <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT>   -c <SOURCE>")
    SET (CMAKE_C_CREATE_SHARED_LIBRARY
         "${SACCC} <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
    SET (CMAKE_C_LINK_EXECUTABLE
         "${SACCC} <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> \
          <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")

    SET (CMAKE_SHARED_LIBRARY_SUFFIX ${SHARED_LIB_EXT})

    ADD_SUBDIRECTORY (${SAC2C_SOURCE_DIR}/${SRC_DIR}    ${SRC_DIR}-${TARGET})
ENDMACRO ()

# convert RT target string back into list
STRING (REPLACE ":" ";" _TARGETS ${RT_TARGETS})
MESSAGE (STATUS "Building the following targets: ${_TARGETS}")

FOREACH (TARGET IN ITEMS ${_TARGETS})
    ADD_LIBSAC_TARGET (src/libsac ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphm ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacphmc ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacdistmem ${TARGET})
    ADD_LIBSAC_TARGET (src/libsac/rtspec ${TARGET})
    ADD_LIBSAC_TARGET (src/libsacprelude ${TARGET})
ENDFOREACH ()

# for tools
ADD_LIBSAC_TARGET (src/tools seq)
