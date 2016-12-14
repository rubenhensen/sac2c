# file include checks to find sac2c, etc.

# Make sure that sac2c is available and is located in the build directory.
# Note the 'NO_DEFAULT_PATH' specifier to the following FIND_PROGRAM command
# which makes sure that any sac2c on the PATH is not considered.
FIND_PROGRAM (SAC2C_EXEC NAMES "sac2c${BUILD_TYPE_POSTFIX}" PATHS ${SAC2C_BUILD_DIR} NO_DEFAULT_PATH)
IF (NOT SAC2C_EXEC)
    MESSAGE (FATAL_ERROR "Could not located the sac2c${BUILD_TYPE_POSTFIX} binary, exiting...")
ENDIF ()
# Check that sac2c actually works by calling "sac2c -V"
EXECUTE_PROCESS (COMMAND ${SAC2C_EXEC} -V RESULT_VARIABLE sac2c_exec_res OUTPUT_QUIET ERROR_QUIET)
IF (NOT "${sac2c_exec_res}" STREQUAL "0")
    MESSAGE (FATAL_ERROR "Call to \"${SAC2C_EXEC} -V\" failed, something "
                         "wrong with the sac2c binary")
ENDIF ()

# FIXME: Are the headers in the correct places?
SET (SAC2C_EXTRA_INC
    -I${SAC2C_BUILD_DIR}/include
    -I${SAC2C_SOURCE_DIR}/include
    -I${SAC2C_SOURCE_DIR}/src/include
    -I${SAC2C_SOURCE_DIR}/src/libsacphm/heap
    -I${SAC2C_SOURCE_DIR}/src/libsacdistmem/commlib)

# add sacprelude file with postfix in module and file name.
CONFIGURE_FILE ("${SAC2C_SOURCE_DIR}/src/libsacprelude/sacprelude.sac" "${PROJECT_BINARY_DIR}/sacprelude${BUILD_TYPE_POSTFIX}.sac" @ONLY)

# Make sure that all the libraries are found here.
SET (DLL_BUILD_DIR "${PROJECT_BINARY_DIR}/lib")
SET (LIBRARY_OUTPUT_PATH "${DLL_BUILD_DIR}")
# Make sure that this directory exists.
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}")
