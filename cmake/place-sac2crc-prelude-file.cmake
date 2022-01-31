# This CMake script is meant to be run directly via CMake's `-P' flag in script
# mode. It creates and copies over a sac2crc file to the user's ~/.sac2crc directory.
#
# It is expected that this script is run from the PROJECT_SOURCE_DIR.
#
# Paramters:
#  - USER_HOME    (required) => path to the user's home directory
#  - SAC2CRC_PATH (required) => name of the sac2crc file to be copied over
#
# Note: all paths *must* be absolute.
# Note(2): this script modifies the state of the user's home directory; checks
#          are in place to prevent bad things from happening, but all due
#          consideration must be given in regards to how this script is used.

# Sanity check. We need to know the user's home directory.
IF (NOT USER_HOME)
    MESSAGE (FATAL_ERROR "User home directory not given")
ELSEIF (NOT EXISTS "${USER_HOME}")
    MESSAGE (FATAL_ERROR "Path does not exists: ${USER_HOME}!")
ELSEIF (NOT IS_ABSOLUTE "${USER_HOME}")
    FILE (REAL_PATH "${USER_HOME}" USER_HOME)
ENDIF ()

# Sanity check. We need to have the name of the sac2crc file.
IF (NOT SAC2CRC_PATH)
    MESSAGE (FATAL_ERROR "Sac2crc file path not given")
ELSEIF (NOT EXISTS "${SAC2CRC_PATH}")
    MESSAGE (FATAL_ERROR "Path does not exists: ${SAC2CRC_PATH}!")
ELSEIF (NOT IS_ABSOLUTE "${SAC2CRC_PATH}")
    FILE (REAL_PATH "${SAC2CRC_PATH}" SAC2CRC_PATH)
ENDIF ()

SET (sac2crc_dir "${USER_HOME}/.sac2crc")

# If the user has no `.sac2crc' directory in their home, we create it.
IF (NOT EXISTS "${sac2crc_dir}")
    MESSAGE (INFO "Creating the `~/.sac2crc' directory")
    FILE (MAKE_DIRECTORY "${sac2crc_dir}")
ELSEIF (NOT IS_DIRECTORY "${sac2crc_dir}")
    MESSAGE (FATAL_ERROR "You are using the old-style sac2crc file,
                          please remove this. Make sure that you
                          preserve any customisation, these can be
                          added seperately to the new sac2crc
                          directory.")
ENDIF ()

MESSAGE (INFO "Copying over sac2crc file")
FILE (INSTALL "${SAC2CRC_PATH}" DESTINATION "${sac2crc_dir}")
