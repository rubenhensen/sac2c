#!/usr/bin/env bash

# Created by SaC Development Team (C) 2016 - 2017

# This is the install script for the SaC Compiler and Runtime libraries.
# It is designed to install/copy the files in the TAR.GZ. release of the
# compiler.
#
# It is assumed that the TAR.GZ. has been extracted.
# 
# This script takes two *required* parameters, the location of the extracted
# tarball files, and the prefix of where to install/copy them over to.

#
# Globals
#

VERSION="1.2"
VERBOSE=0

#
# Helpers
#

verbose() { if [[ $VERBOSE = 1 ]]; then echo -e "[VERB]: ${@}"; fi }
warning() { echo -e "[WARN]: ${@}" 1>&2; }
error() { echo -e "[ERROR]: ${@}" 1>&2; }
fatal_error() { error "${@}"; exit 10; }
usage() { echo "Usage: $0 [-h] [-v] [-V] [-l LOCATION] [-p PREFIX]" 1>&2; }
help_msg() {
  cat <<EOF >&2

  Options:
    -h            Print this help message and exit
    -v            Display verbose messages
    -V            Print version and exit
    -l LOCATION   Specify location of SaC compiler files
    -p PREFIX     Specify location to install to
EOF
}

#
# Argument Parsing
#

while getopts "hvVl:p:" o; do
  case "${o}" in
    v) # activate verbose printing
      VERBOSE=1
      ;;
    V)
      echo "$0 $VERSION (c) SaC Developer Team"
      exit 0
      ;;
    l)
      location=${OPTARG}
      ;;
    p)
      prefix=${OPTARG}
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
  echo "No options passed..." 1>&2
  usage
fi
shift $((OPTIND-1))

if [[ "z$location" = "z" || "z$prefix" = "z" ]]; then
  fatal_error "-----------------------------------\n" \
              "Missing parameters for the location of the\n" \
              "SaC binaries and/or the install prefix. These\n" \
              "can be set with -l and -p, respectively\n"
fi

#
# Main Procedure
#

# check if the install location exists, and is writeable to the user
if [[ -d "$prefix" ]]; then
  if [[ ! -r "$prefix" && ! -w "$prefix" ]]; then
    fatal_error "\`$prefix' is not writeable for the current user (do you" \
      "need root-access?)."
  fi
else
  fatal_error "\`$prefix' does not seem to exist. Please create it."
fi

if [[ ! -d "$location" ]]; then
  fatal_error "\`$location' does not exist, is this where you extracted the tarball?"
fi

verbose "$location exists"

if [[ ! -d "$location/src" ]]; then
  fatal_error "No source directory found"
fi

verbose "Found source dir"

count=$(ls -l "$location/src" | wc -l)
if [[ $count == 0 ]]; then
  fatal_error "No source files found in \`$location/src'"
fi

verbose "Found source files"

if [[ ! -w "$location/src/sacdirs.h" ]]; then
  fatal_error "Unable to access \`sacdirs.h' file, is it missing?"
fi

# determin sac compiler version
SAC_VERSION="$(awk '/SAC2C_VERSION/{ gsub(/"/,"",$3); print $3 }' "$location/src/sacdirs.h")"

verbose "Determined compiler version is \`$SAC_VERSION'"

# determine postfix for binaries
binpostfix="$(find . -type f -name '*_[pd]' | grep -o '_[dp]$' | uniq)"
if [[ "x$binpostfix" == "x" || $(echo $binpostfix | wc -l) > 1 ]]; then
  fatal_error "Incorrect build type postfix found: \`$binpostfix'"
fi

verbose "Determined binary postfix is \`$binpostfix'"

dirs=("share" "include" "lib" "libexec")
for dir in "${dirs[@]}"; do
  fdir="$location/$dir"
  # confirm that the dir exists and layout is correct
  if [[ ! -d "$fdir" ]]; then
    fatal_error "Missing \`$dir' in $location"
  elif [[ ! -d "$fdir/sac2c" ]]; then
    fatal_error "Missing \`sac2c' in $fdir"
  fi

  # make sure we have only one dir under $fdir/sac2c
  $(ls -l "$fdir/sac2c" | grep "$SAC_VERSION" >/dev/null)
  if [[ $? > 0 ]]; then
    fatal_error "Wrong version found in $fdir/sac2c. Something is wrong with your tarball extraction..."
  fi
done

# determine shared library extension
_sharedlibext="$(find . -type f -name 'libsac2c*' -exec basename {} \;)"
sharedlibext=".${_sharedlibext##*.}"
if [[ "x$sharedlibext" == "x" ]]; then
  fatal_error "Unable to determine shared library extension"
fi

verbose "Shared library extension is: \`$sharedlibext'"

# we now need to update sacdir.h, we assume not to know the current
# prefix, but do assume that it is the same for both SAC2CRC_DIR and DLL
cprefix="$(awk '/SAC2CRC_DIR/{ sub(/\/share.*/,"",$3); gsub(/"/,"",$3); print $3 }' "$location/src/sacdirs.h")"
if [[ "x$cprefix" == "x" ]]; then
  fatal_error "Unable to read sacdirs.h file"
fi

verbose "Found old prefix: $cprefix"

if [[ "x$prefix" != "x$cprefix" ]]; then
  # do an inplace replacement
  sed -e "s|$cprefix|$prefix|" -i "$location/src/sacdirs.h"

  # and lets not for get to update the sac2crc file
  sed -re "s|$cprefix(.*sac2c.*)|$prefix\1|" -i "$location/share/sac2c/$SAC_VERSION/sac2crc$binpostfix"

  verbose "Updated sacdirs.h and sac2crc files with new prefix: $prefix"
fi

# create tmp makefile with targets
tmpfile=$(mktemp)
# ensure we delete the tmpfile when anything happens - including
# exiting the script correctly.
trap "rm -f -- $tmpfile" INT TERM HUP EXIT
stab=$'\t' # hackish way of getting tabs into heredoc

# FIXME: there might be a better way of doing this... 
cat <<-WHATWHAT > "$tmpfile"
CFLAGS+= -I. -DBUILD_TYPE_POSTFIX=\"$binpostfix\" -DSHARED_LIB_EXT=\"$sharedlibext\"
LDFLAGS+= -ldl
all: sac2c$binpostfix sac4c$binpostfix sac2tex$binpostfix
sac2c$binpostfix: sac2c.c sactools.h sacdirs.h
$stab\$(CC) \$(CFLAGS) \$(LDFLAGS) \$^ -o \$@
sac4c$binpostfix: sac4c.c sactools.h sacdirs.h
$stab\$(CC) \$(CFLAGS) \$(LDFLAGS) \$^ -o \$@
sac2tex$binpostfix: sac2tex.c sactools.h sacdirs.h
$stab\$(CC) \$(CFLAGS) \$(LDFLAGS) \$^ -o \$@
WHATWHAT

verbose "Created temp Makefile, now building"

# run make
make -C "$location/src" -f "$tmpfile"
if [[ $? != 0 ]]; then
  exit $?;
fi

verbose "Completed making binaries"

verbose "Installing binaries and libraries to \`$prefix'"

# first we do libraries
for dir in "${dirs[@]}"; do
  fdir="$location/$dir"
  cp -r "$fdir" "$prefix"
  if [[ $? != 0 ]]; then
    fatal_error "Copying of \`$fdir' failed"
  fi
done

# then we do binaries
files=($location/src/*_*)
cp "${files[@]}" "$prefix/libexec/sac2c/$SAC_VERSION"
if [[ $? != 0 ]]; then
  fatal_error "Copying of binaries to \`$prefix/libexec/sac2c/$SAC_VERSION' failed"
fi

echo "The install is complete"

exit 0
# vim: ts=2 sw=2 et:
