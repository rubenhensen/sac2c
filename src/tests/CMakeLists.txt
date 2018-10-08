# Make sure that we Have GTest, and add some boilerplate definitions.
FIND_PACKAGE (GTest REQUIRED)
INCLUDE_DIRECTORIES (${GTEST_INCLUDE_DIR})

# As we are going to interface libsac2c.so, we want to define all the
# include paths, to avoid writing function declarations manually.
FILE (GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/src/libsac2c/*.h)
SET (dirlist "")
FOREACH (hpath ${headers})
   GET_FILENAME_COMPONENT (hdir ${hpath} PATH)
   LIST (APPEND dirlist ${hdir})
ENDFOREACH ()
LIST (REMOVE_DUPLICATES dirlist)

# All the src/libsac2c subdirs that contain *.h
INCLUDE_DIRECTORIES (${dirlist})

# All the generated files
INCLUDE_DIRECTORIES (${PROJECT_BINARY_DIR}/src/libsac2c)

# Generated global files like config, fun-attrs, etc.
INCLUDE_DIRECTORIES (${PROJECT_BINARY_DIR}/src/include)

# Generated dummy runtime library,includes dummy functions to satisfy linker
ADD_LIBRARY (runtimedummy STATIC "runtime_dummy.c")

# Some general variables
SET (SAC2C_TESTS_PATH "${PROJECT_SOURCE_DIR}/src/tests")

# FIXME (hans)
# we need to be super carefully to have actually build the compiler/libs *before*
# we call ctest/`make test` as seemingly there is no implicit dependency between
# the two stages. It is completely possible for ctest to be called after configure
# and exit cleanly (missing tests do not cause an error!).
# Issue is dealt with and reported here: https://stackoverflow.com/q/733475 and
# https://gitlab.kitware.com/cmake/cmake/issues/8774.

MACRO (ADD_FUNC_TEST BIN_NAME SRC_NAME)
    # Compile the test
    ADD_EXECUTABLE (${BIN_NAME} ${SRC_NAME})
    # Link against Gtest
    TARGET_LINK_LIBRARIES (${BIN_NAME} GTest::GTest GTest::Main sac2cShared)
    # Make it a part of the global testsuite so that it is executed via ctests.
    GTEST_ADD_TESTS (TARGET ${BIN_NAME})
ENDMACRO ()

MACRO (ADD_RUNTIME_FUNC_TEST BIN_NAME SRC_NAME LIB_TARGET EXTRA_LINKS)
    # Compile the test
    ADD_EXECUTABLE (${BIN_NAME} ${SRC_NAME})
    SET (__runtimelibpath "${PROJECT_BINARY_DIR}/runtime_build/src/runtime_libraries-build/lib/host/${LIB_TARGET}")
    # pass information to the binary
    SET_TARGET_PROPERTIES (${BIN_NAME}
        PROPERTIES
        COMPILE_DEFINITIONS "CMAKE_TESTS_PATH=\"${SAC2C_TESTS_PATH}\""
        BUILD_RPATH "${__runtimelibpath}")
    # Link against Gtest and SaC runtime library (we need to link manually as the ExternalProject does not encode this information)
    TARGET_LINK_LIBRARIES (${BIN_NAME} GTest::GTest GTest::Main "${__runtimelibpath}/libsac${BUILD_TYPE_POSTFIX}${SHARED_LIB_EXT}" runtimedummy ${EXTRA_LINKS})
    # Make it a part of the global testsuite so that it is executed via ctests.
    GTEST_ADD_TESTS (TARGET ${BIN_NAME})
ENDMACRO ()

# libsac2c tests
ADD_FUNC_TEST (string-tests string-tests.cpp)
ADD_FUNC_TEST (test-assoc-law test-assoc-law.cpp)
ADD_FUNC_TEST (test-icm-compilation test-icm-compilation.cpp)

# libsac + runtime tests
# XXX (hans) we can only create one test suite, *not* per-target, due to name-clashes
IF (${ENABLE_HWLOC})
    ADD_RUNTIME_FUNC_TEST (test-hwloc-runtime test-hwloc-runtime.cpp "seq" "hwloc")
ENDIF ()
