# Create the list of directories.
FILE (GLOB tmplst RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" *)
SET (DIR_LIST)
FOREACH (dir ${tmplst})
  SET (mask "CMakeLists.txt|Makefile|m4")
  IF (NOT ${dir} MATCHES ${mask})
    LIST (APPEND DIR_LIST ${dir})
  ENDIF (NOT ${dir} MATCHES ${mask})
ENDFOREACH (dir)

# Create a list of directories in the project binary dir
# in order to generate .h.m4 files.
FOREACH (dir ${DIR_LIST})
  FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
ENDFOREACH (dir)

SET (M4_FILES
    cuda_h/cuda_gen.h
    essentials_h/rc_impl_gen.h
    essentials_h/std_gen.h
    mt_h/mt_gen.h
    mt_h/fp_gen.h
    mutc_h/mutc_gen.h
    mutc_h/mutc_rc_gen.h
    nested_h/nested_gen.h
)

SET (M4_HEADERS)
FOREACH(name ${M4_FILES})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}.m4")
  SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"  "${CMAKE_CURRENT_SOURCE_DIR}/m4/icm.m4"
    COMMAND ${M4_EXEC} -I"${CMAKE_CURRENT_SOURCE_DIR}/m4" ${src} > ${dst}
    COMMENT "Generating ${dst}"
  )
  LIST (APPEND M4_HEADERS ${dst})
ENDFOREACH (name)

FILE (GLOB_RECURSE RUNTIME_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
SET (CMAKE_LIBSAC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libsac")
FILE (GLOB_RECURSE LIBSAC_HEADERS RELATIVE "${CMAKE_LIBSAC_SOURCE_DIR}" "${CMAKE_LIBSAC_SOURCE_DIR}/*.h")

# XXX(artem) we can avoid copying this file by adding -I./include/
#            to default flags.  Not sure whether it would brake something,
#            so not doig it right now (2018/10).
SET (SAC_H  "${PROJECT_BINARY_DIR}/include/sac.h")
ADD_CUSTOM_COMMAND(
  OUTPUT "${SAC_H}"
  DEPENDS "${PROJECT_SOURCE_DIR}/include/sac.h"
  COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/include/sac.h" ${SAC_H}
  COMMENT "Copying sac.h"
)


# copy RUNTIME_HEADERS into ${PROJECT_BINARY_DIR}/include/runtime
SET (RUNTIME_HEADERS_DONE)
FOREACH (name ${RUNTIME_HEADERS})
  SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/runtime/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND RUNTIME_HEADERS_DONE ${dst})
ENDFOREACH (name)

# copy M4_HEADERS headers into ${PROJECT_BINARY_DIR}/include/runtime
SET (M4_HEADERS_DONE)
FOREACH (name ${M4_FILES})
  SET(src "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/runtime/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND M4_HEADERS_DONE ${dst})
ENDFOREACH (name)

# copy LIBSAC_HEADERS into ${PROJECT_BINARY_DIR}/include/libsac

SET (LIBSAC_HEADERS_DONE)
FOREACH (name ${LIBSAC_HEADERS})
  SET(src "${CMAKE_LIBSAC_SOURCE_DIR}/${name}")
  SET(dst "${PROJECT_BINARY_DIR}/include/libsac/${name}")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${dst}"
    DEPENDS "${src}"
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
    COMMENT "Copying header to ${dst}"
  )
  LIST (APPEND LIBSAC_HEADERS_DONE ${dst})
ENDFOREACH (name)

ADD_CUSTOM_TARGET (sac_h ALL DEPENDS ${SAC_H} ${RUNTIME_HEADERS_DONE} ${M4_HEADERS_DONE} ${LIBSAC_HEADERS_DONE})


# install include files
XINSTALL (
  DIRECTORY "${PROJECT_BINARY_DIR}/include/runtime"
            "${PROJECT_BINARY_DIR}/include/libsac"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)
XINSTALL (FILES "${SAC_H}"
  DESTINATION "${INCPATH_INSTALL}"
  COMPONENT headers)
