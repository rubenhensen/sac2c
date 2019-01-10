#!/usr/bin/env bash

# Created by the SaC Development Team (C) 2016 - 2019

# This script is a wrapper around the installer scripts, the installer
# scripts are designed to install the SaC libraries and compile the binaries
# from source and install these as well. This wrapper allows us to call
# all the installer scripts in one go.
#
# For more information, have a look at the comments in the installer scripts.

#
# Globals
#

# this is left intentionally uninitialised
INSTALLDIR=
SUBARGS=()

#
# Helper functions
#

msg() { echo "> ${*}"; }
msg2() { echo "==> ${*}"; }
usage() { echo "Usage: $0 [-h] -i INSTALL [-- [ARGS]]" 1>&2; }
# FIXME (hans) update link in description...
help_msg() {
  cat <<EOF >&2

  This is a simple wrapper to launch the installer scripts
  for the different variants of the SaC2C compiler.

  Options:
    -h            Print this help message and exit
    -i INSTALL    Specify the installation location

    -- ARGS       All arguments given here are passed on
                  to the sub-installer-script. Check the
                  supported options first in the script.

  For more information: See www.sac-home.org/downloads
EOF
}

#
# Functions
#

## Argument Parsing
argparse()
{
  local idir=

  while getopts "hi:" o; do
    case "${o}" in
      i)
        idir="$OPTARG"
        ;;
      h)
        usage
        help_msg
        exit 0
        ;;
      \?) # if we pass any flag that is unknown
        usage
        exit 1
        ;;
    esac
  done

  # if no options were passed, we just show a message and exit
  if [[ $OPTIND == 1 ]]; then
    msg "No options passed..." 1>&2
    usage
    exit 0
  fi

  # fix args position
  shift $((OPTIND-1))

  # collect remaing args
  SUBARGS+=("$@")

  if [[ "z$idir" = "z" ]]; then
    msg "Install directory is empty!" 1>&2
    exit 10
  fi

  INSTALLDIR="$idir"
}

#
# Main Procedure
#

argparse "${@}"

# We assume that this script will only be used from the extracted archive
# directory!
if [[ ! -d "$PWD/installers" ]]; then
  msg "This script is not being run from withing" \
    " the SaC extracted archive directory!" >&2
  exit 10
fi

# We get the list of installers, and run them
INSTALLERS=("$PWD"/installers/*.sh)
for installer in ${INSTALLERS[*]}; do
  msg "Running installer \`$installer'"
  if ! bash "$installer" -i "$INSTALLDIR" -s "$PWD" "${SUBARGS[@]}"; then
    msg2 "An error occurred, aborting..." 1>&2
    exit 12
  fi
done

msg "Done"

exit 0

# vim: ts=2 sw=2 et:
