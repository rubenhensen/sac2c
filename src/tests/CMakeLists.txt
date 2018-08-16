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

MACRO (ADD_FUNC_TEST BIN_NAME SRC_NAME)
    # Compile the test
    ADD_EXECUTABLE (${BIN_NAME} ${SRC_NAME})
    # Link against Gtest
    TARGET_LINK_LIBRARIES (${BIN_NAME} GTest::GTest GTest::Main sac2cShared)
    # Make it a part of the global testsuite
    # so that it is executed via ctests.
    GTEST_ADD_TESTS (${BIN_NAME} "" AUTO)  # maket it a part of ctest call
ENDMACRO ()

ADD_FUNC_TEST (string-tests string-tests.cpp)
ADD_FUNC_TEST (test-assoc-law test-assoc-law.cpp)
ADD_FUNC_TEST (test-icm-compilation test-icm-compilation.cpp)
