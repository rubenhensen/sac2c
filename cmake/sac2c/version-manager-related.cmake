# configure version-manager script 
SET (BUILD_TYPE_DICTIONARY)
FOREACH (t ${KNOWN_BUILD_TYPES})
  STRING (APPEND BUILD_TYPE_DICTIONARY "'${t}': '${CMAKE_${t}_POSTFIX}',")
ENDFOREACH ()
SET (BUILD_TYPE_DICTIONARY "{ ${BUILD_TYPE_DICTIONARY} }")

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/scripts/sac2c-version-manager.in"
  "${PROJECT_BINARY_DIR}/scripts/sac2c-version-manager" @ONLY)
