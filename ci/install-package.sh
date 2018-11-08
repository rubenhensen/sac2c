#/usr/bin/env bash

# Created by the SaC Development Team (C) 2018

# exit with return of failed subcommand
set -e

# will install packages using platform specific tools:
#   DEB/RPM/TGZ/MAC

# we determine which platform we based on both OSTYPE and the extension of the
# first argument. So "*.tar.gz" should be installed using `tar`, whereas
# "*.deb" should be installed using `apt-get`.

# global option(s)
ISSTDLIB=0 # false

# functions
error() { echo -e "[ERROR]: ${@}" >&2; }
fatal_error() { error "${@}"; exit 1; }
usage () { echo "Usage: $0 [-S] file" >&2; }

# parse arguments
while getopts "S" o; do
    case "${o}" in
        S)
            ISSTDLIB=1
            ;;
        \?)
            usage
            exit 1
            ;;
    esac
done

# fix args position
shift $((OPTIND-1))

# if no file was passed, we just show a message and exit
if [[ $# == 0 ]]; then
  error "No file passed..."
  usage
  exit 1
fi

# unpack/install
case "$1" in
    *tar.gz)
        if [[ $ISSTDLIB == 0 ]]; then
            mkdir tarout && cd tarout
            tar -xzf "$1"
            bash ./install.sh -i "/usr/local" -- -v -S
            cd .. && rm -rf tarout
        else
            # there is no special handling of stdlib tar needed
            # just extract :)
            tar -C "/" -xzf "$1"
        fi
        ;;
    *deb)
        dpkg -i "$1"
        ;;
    *rpm)
        #FIXME what do we do on systems like fedora which uses DNF?
        yum install -y "$1"
        ;;
    *pkg)
        if [[ "$OSTYPE" != "darwin"* ]]; then
            echo "Need to be on MacOSX to install $1."
            exit 100;
        else
            echo "Not yet implemented, any takers :)?"
            exit 1
        fi
        ;;
    *)
        echo "That install mechanism isn't supported! Exiting..."
        exit 100
        ;;
esac
