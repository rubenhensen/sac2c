## CMake Script

##
# Parameters
#  -D OUTPUT=<absolute path>       (Mandatory)
#  -D REL_HEADERS=<list of relative paths from sac-runtime> (Mandatory)
#  -D REL_LIB_HEADERS=<list of relative paths from sac-libsac> (Mandatory)
##

IF (NOT (DEFINED OUTPUT AND DEFINED REL_HEADERS))
  MESSAGE (SEND_ERROR "Did you forgot to pass the right parameters?")
  RETURN ()
ENDIF ()

FILE( WRITE "${OUTPUT}" "/* auto generated via cmake */\n")
STRING( REPLACE " " ";" HEADER_LIST ${REL_HEADERS})
FOREACH( name ${HEADER_LIST})
  FILE(APPEND "${OUTPUT}" "#include \"runtime/${name}\"\n")
ENDFOREACH( name)
STRING( REPLACE " " ";" HEADER_LIST ${REL_LIB_HEADERS})
FOREACH( name ${HEADER_LIST})
  FILE(APPEND "${OUTPUT}" "#include \"libsac/${name}\"\n")
ENDFOREACH( name)
FILE(APPEND "${OUTPUT}" "\n")


