IF (CPACK_GENERATOR MATCHES "TGZ")
  SET (CPACK_COMPONENTS_ALL rtapplications libraries sources headers config symlinks)
  SET (CPACK_PACKAGING_INSTALL_PREFIX  "")
ELSE ()
  SET (CPACK_COMPONENTS_ALL applications rtapplications libraries headers config symlinks)
  SET (CPACK_PACKAGING_INSTALL_PREFIX  "${CMAKE_INSTALL_PREFIX}")
ENDIF ()
