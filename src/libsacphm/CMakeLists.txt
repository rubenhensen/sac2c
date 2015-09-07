# Create the list of directories.
FILE (GLOB tmplst RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" *)
SET (DIR_LIST)
FOREACH (dir ${tmplst})
  SET (mask "CMakeLists.txt|Makefile|xml")
  IF (NOT ${dir} MATCHES ${mask})
    LIST (APPEND DIR_LIST ${dir}) 
  ENDIF (NOT ${dir} MATCHES ${mask})
ENDFOREACH (dir)

# Add each directory to include_dirs.
FOREACH (d ${DIR_LIST})
  INCLUDE_DIRECTORIES ("${CMAKE_CURRENT_SOURCE_DIR}/${d}")
  FILE (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${d}")
ENDFOREACH (d)

SET (PHM_SRC
  diag/diagnostics.c
  heap/large_chunks.c
  heap/wrapper.c
  heap/setup.c
  heap/small_chunks.c
  heap/internal.c
  opt/placement.c
  compat/malloc.c
)


# Create sources for .mt.pth variant of the library.
SET (PHM_SRC_MT)
FOREACH (phm_file ${PHM_SRC})
  SET (dst "${CMAKE_CURRENT_BINARY_DIR}/${phm_file}.mt.pth.c")
  CONFIGURE_FILE("${phm_file}" "${dst}" COPYONLY)
  LIST(APPEND PHM_SRC_MT "${dst}")
ENDFOREACH ()
SET_SOURCE_FILES_PROPERTIES(${PHM_SRC_MT} 
  PROPERTIES COMPILE_FLAGS "-D_REENTRANT -DMT -DPTH"
)


SET (LIB_PATH "${PROJECT_SOURCE_DIR}/lib")

ADD_LIBRARY (phmShared SHARED ${PHM_SRC})
ADD_LIBRARY (phmStatic STATIC ${PHM_SRC})
ADD_DEPENDENCIES (phmStatic sac_h)
ADD_DEPENDENCIES (phmShared sac_h)
SET_TARGET_PROPERTIES (phmStatic 
    PROPERTIES OUTPUT_NAME sacphm
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH}
)
SET_TARGET_PROPERTIES (phmShared
    PROPERTIES OUTPUT_NAME sacphm
    LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH}
)


ADD_LIBRARY (phmMtShared SHARED ${PHM_SRC_MT})
ADD_LIBRARY (phmMtStatic STATIC ${PHM_SRC_MT})
ADD_DEPENDENCIES (phmMtStatic sac_h)
ADD_DEPENDENCIES (phmMtShared sac_h)
SET_TARGET_PROPERTIES (phmMtStatic
    PROPERTIES OUTPUT_NAME sacphm.mt.pth
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH}
)
SET_TARGET_PROPERTIES (phmMtShared
    PROPERTIES OUTPUT_NAME sacphm.mt.pth
    LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH}
)

