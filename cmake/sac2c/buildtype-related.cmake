# This files checks whether the variables that we use during the
# configure/build process that we derive from CMAKE_BUILD_TYPE
# are consistent.


FUNCTION (BUILD_TYPE_TO_DIRNAME BT DIRNAME)
    STRING (TOLOWER "${BT}" BT_DIR)
    SET (${DIRNAME} ${BT_DIR} PARENT_SCOPE)
ENDFUNCTION ()


# 1. Check whether the given <build-type> is supported, i.e.
#    it can be found in KNOWN_BUILD_TYPES
IF (NOT "${CMAKE_BUILD_TYPE}" IN_LIST KNOWN_BUILD_TYPES)
  MESSAGE (FATAL_ERROR 
             "The build type `${CMAKE_BUILD_TYPE}' is unknown. "
             "Please add it to KNOWN_BUILD_TYPES in "
             "`cmake/sac2c/config.cmake'")
ENDIF ()

# 2. Check whther the postfix of the given <build-type> is defined
# FIXME(artem) check that postfixes are different!
IF (NOT CMAKE_${CMAKE_BUILD_TYPE}_POSTFIX)
  MESSAGE (FATAL_ERROR 
             "The postfix for the binaries for the build type "
             "`${CMAKE_BUILD_TYPE}' is not defined.  Please set "
             "CMAKE_${CMAKE_BUILD_TYPE}_POSTFIX in "
             "`cmake/sac2c/config.cmake'")
ENDIF ()
                       
# 3. Check that <postfix> for the given <build-type> is defined
IF (NOT CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE})
  MESSAGE (FATAL_ERROR 
             "The C flags for the build type ${CMAKE_BUILD_TYPE} "
             "are not defined.  Please set "
             "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE} variable in "
             "`cmake/sac2c/config.cmake'")
ENDIF ()
