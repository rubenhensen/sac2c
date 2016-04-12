
# Find subdirectories of the given directory that contain *.h files.
MACRO (HEADER_DIRECTORIES ret)
    FILE (GLOB_RECURSE headers *.h)
    SET (dirlist "")
    FOREACH (hpath ${headers})
        GET_FILENAME_COMPONENT (hdir ${hpath} PATH)
        LIST (APPEND dirlist ${hdir})
    ENDFOREACH ()
    LIST (REMOVE_DUPLICATES dirlist)
    SET (${ret} ${dirlist})
ENDMACRO ()


# Implement OS-independent "date" command for retreiving current date and time.
function (CURRENT_TIME var)
    set (t "${PROJECT_SOURCE_DIR}/__timestamp_file")
    file (WRITE "${t}"  "x")
    file (TIMESTAMP ${t} curtime UTC)
    file (REMOVE ${t})
    set (${var} ${curtime} PARENT_SCOPE)
endfunction ()


# Get current user.
MACRO (GET_USERNAME RES)
  SET (${RES} "<unknown-user>")
  IF (WIN32)
    SET (${RES} $ENV{USERNAME})
  ELSE ()
    SET (${RES} $ENV{USER})
  ENDIF ()
ENDMACRO ()

MACRO (ASSERT_LIB VAR lib_name lib_func)
    FIND_LIBRARY (${VAR}_LIB      "${lib_name}")
    IF (${VAR}_LIB)
        GET_FILENAME_COMPONENT (${VAR}_PATH  ${${VAR}_LIB} PATH)
        CHECK_LIBRARY_EXISTS (${${VAR}_LIB} "${lib_func}" ${${VAR}_PATH} ${VAR}_FOUND)
    ENDIF ()

    IF (NOT ${VAR}_FOUND OR NOT ${VAR}_LIB)
        MESSAGE (FATAL_ERROR "Cannot find '${lib_name}' library which must be present to compile sac2c")
    ENDIF ()
ENDMACRO ()

# Check if compiler `flag' is supported, and if so append it to the `var' string.
MACRO (CHECK_GCC_FLAG flag var)
    SET (OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    SET (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror ${flag}")
    # Generate the name of the check as it is cached
    STRING (REPLACE "-" "" FLAG "${flag}")
    STRING (REPLACE "=" "" FLAG "${FLAG}")
    STRING (TOUPPER ${FLAG} FLAG)
    CHECK_C_SOURCE_COMPILES ("int main (void) {return 0;}" GCC_HAS_${FLAG}_FLAG)
    IF (${GCC_HAS_${FLAG}_FLAG})
        #MESSAGE (STATUS "${flag} is supported by GCC")
        SET (${var} "${${var}} ${flag}")
    ENDIF ()
    SET (CMAKE_REQUIRED_FLAGS "${OLD_CMAKE_REQUIRED_FLAGS}")
ENDMACRO ()

# Generate version using `git describe'
# FIXME(artem) what happens if we are compiling from withing the source 
# distribution and .git directory is not available?  We need to have a
# mechanism to deal with this situation.  For example checking for SAC2C_VERSION
# file in case .git directory is not present.
FUNCTION (GET_VERSION ver)
    FIND_PACKAGE (Git)

    IF (Git_FOUND)
        EXECUTE_PROCESS (COMMAND
            "${GIT_EXECUTABLE}"
            describe
            --tags 
            --abbrev=4
            --dirty
            WORKING_DIRECTORY
                "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE
                res
            OUTPUT_VARIABLE
                out
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        IF (NOT res EQUAL 0)
            MESSAGE (FATAL_ERROR "Failing to get git version")
        ENDIF ()
        STRING (REGEX REPLACE "^v" "" out "${out}")
        SET (${ver} "${out}" PARENT_SCOPE)
    ELSE ()
        MESSAGE (FATAL_ERROR "Git executable not found")
    ENDIF ()
ENDFUNCTION ()
