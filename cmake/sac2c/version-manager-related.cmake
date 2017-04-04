# configure version-manager script 

# create a dictionary that relates build type and the build postfix
SET (BUILD_TYPE_POSTFIX_DICT)
FOREACH (t ${KNOWN_BUILD_TYPES})
  STRING (APPEND BUILD_TYPE_POSTFIX_DICT "'${t}': '${CMAKE_${t}_POSTFIX}',")
ENDFOREACH ()
SET (BUILD_TYPE_POSTFIX_DICT "{ ${BUILD_TYPE_POSTFIX_DICT} }")

# create a dictionary that relates build type and the name of the directory
# used during the installation process
SET (BUILD_TYPE_DIRECTORY_DICT)
FOREACH (t ${KNOWN_BUILD_TYPES})
  BUILD_TYPE_TO_DIRNAME (${t} dirname)
  STRING (APPEND BUILD_TYPE_DIRECTORY_DICT "'${t}': '${dirname}',")
ENDFOREACH ()
SET (BUILD_TYPE_DIRECTORY_DICT "{ ${BUILD_TYPE_DIRECTORY_DICT} }")



CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/scripts/sac2c-version-manager.in"
  "${PROJECT_BINARY_DIR}/scripts/sac2c-version-manager" @ONLY)
