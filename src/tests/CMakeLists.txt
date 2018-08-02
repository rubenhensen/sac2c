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

ADD_EXECUTABLE (string-tests string-tests.cpp)
TARGET_LINK_LIBRARIES (string-tests GTest::GTest GTest::Main sac2cShared)
GTEST_ADD_TESTS(string-tests "" AUTO)  # maket it a part of ctest call
