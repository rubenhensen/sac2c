
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
MACRO (CHECK_CC_FLAG flag var)
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

# Do variable substitution like `configure_file' but without
# attaching to the configure stage
MACRO (SUBSTITUTE_FILE infile outfile)
  FILE (READ "${infile}" infilev)
  MESSAGE (STATUS "Parsing ${infile}")
  STRING (CONFIGURE "${infilev}" parsed @ONLY)
  MESSAGE (STATUS "Writing ${outfile}")
  FILE (WRITE "${outfile}" "${parsed}")
ENDMACRO ()

# vim: ts=2 sw=2 et:
