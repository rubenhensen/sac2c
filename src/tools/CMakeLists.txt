INCLUDE ("${SAC2C_SOURCE_DIR}/cmake/runtime/sac2c-variables.cmake")

# create directory
FILE (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csima")
FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csimt")
SET (BINDIR "${PROJECT_BINARY_DIR}/bin")

ADD_CUSTOM_COMMAND (
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/csima/csima.o"
  DEPENDS libsacprelude-seq "${CMAKE_CURRENT_SOURCE_DIR}/csima/csima.c" ${LIB_SAC2C}
  COMMAND ${SAC2C_NT} -target seq -v0 -cc ccprog
          -o "${CMAKE_CURRENT_BINARY_DIR}/csima/csima.o" "${CMAKE_CURRENT_SOURCE_DIR}/csima/csima.c"
  COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/csima/csima.o")

ADD_CUSTOM_COMMAND (
  OUTPUT "${BINDIR}/csima${BUILD_TYPE_POSTFIX}"
  COMMAND ${SAC2C_NT} -target seq -L "${DLL_BUILD_DIR}" -v0 -cc ldprog ${LIB_M} ${LIB_RT}
          -o "${BINDIR}/csima${BUILD_TYPE_POSTFIX}" "${CMAKE_CURRENT_BINARY_DIR}/csima/csima.o"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csima/csima.o"
  COMMENT "Linking for `csima' program")
ADD_CUSTOM_TARGET (csima-prog ALL DEPENDS "${BINDIR}/csima${BUILD_TYPE_POSTFIX}")

ADD_CUSTOM_COMMAND (
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/csimt/csimt.o"
  DEPENDS libsacprelude-seq "${CMAKE_CURRENT_SOURCE_DIR}/csimt/csimt.c" "${LIB_SAC2C}"
  COMMAND ${SAC2C_NT} -target seq -v0 -cc ccprog
          -o "${CMAKE_CURRENT_BINARY_DIR}/csimt/csimt.o" "${CMAKE_CURRENT_SOURCE_DIR}/csimt/csimt.c"
  COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/csimt/csimt.o"
)

ADD_CUSTOM_COMMAND (
  OUTPUT "${BINDIR}/csimt${BUILD_TYPE_POSTFIX}"
  COMMAND ${SAC2C_NT} -target seq
          -L "${DLL_BUILD_DIR}" -v0 -cc ldprog ${LIB_M} ${LIB_RT}
          -o "${BINDIR}/csimt${BUILD_TYPE_POSTFIX}" "${CMAKE_CURRENT_BINARY_DIR}/csimt/csimt.o"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/csimt/csimt.o"
  COMMENT "Linking for `csimt' program")
ADD_CUSTOM_TARGET (csimt-prog ALL DEPENDS "${BINDIR}/csimt${BUILD_TYPE_POSTFIX}")
