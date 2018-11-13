# based on https://gist.github.com/johnb003/65982fdc7a1274fdb023b0c68664ebe4

# this is the last stable version before C++11 support is mandatory
# we use 1.8.1 as it runs on old systems with older compilers
# we don't use GMOCK, and compiling it needs C++11 support, so we disable it.
ExternalProject_Add (googletest
    URL               https://github.com/google/googletest/archive/release-1.8.1.tar.gz
    URL_HASH          SHA1=152b849610d91a9dfa1401293f43230c2e0c33f8
    CMAKE_ARGS        -DBUILD_GMOCK=OFF
    UPDATE_COMMAND    ""
    INSTALL_COMMAND   ""
)

ExternalProject_Get_Property (googletest source_dir)
SET (GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)

# The cloning of the above repo doesn't happen until make, however if the dir doesn't
# exist, INTERFACE_INCLUDE_DIRECTORIES will throw an error.
# To make it work, we just create the directory now during config.
FILE (MAKE_DIRECTORY ${GTEST_INCLUDE_DIRS})

ExternalProject_Get_Property(googletest binary_dir)
SET (GTEST_LIBRARY_PATH ${binary_dir}/googletest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
SET (GTEST_LIBRARY gtest)
ADD_LIBRARY (${GTEST_LIBRARY} UNKNOWN IMPORTED)
SET_TARGET_PROPERTIES (${GTEST_LIBRARY} PROPERTIES
    "IMPORTED_LOCATION" "${GTEST_LIBRARY_PATH}"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIRS}")
ADD_DEPENDENCIES (${GTEST_LIBRARY} googletest)

SET (GTEST_MAIN_LIBRARY_PATH ${binary_dir}/googletest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
SET (GTEST_MAIN_LIBRARY gtest_main)
ADD_LIBRARY (${GTEST_MAIN_LIBRARY} UNKNOWN IMPORTED)
SET_TARGET_PROPERTIES (${GTEST_MAIN_LIBRARY} PROPERTIES
    "IMPORTED_LOCATION" "${GTEST_MAIN_LIBRARY_PATH}"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIRS}")
ADD_DEPENDENCIES (${GTEST_MAIN_LIBRARY} googletest)
