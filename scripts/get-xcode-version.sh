#!/usr/bin/env bash

# this script gets the version of XCode currently 'activated'
# in the environment using `xcodebuild`.

if [ ! -x "/usr/bin/xcodebuild" ]; then
    echo "ERROR: XCode (and command line tools) doesn't seem to be installed!" >&2
    exit 1
fi

# the output from -version is `XCode VERSION`, we return only the second field
/usr/bin/xcodebuild -version 2>/dev/null | /usr/bin/grep -E "[0-9]+\.[0-9]+\.[0-9]+" | /usr/bin/cut -d' ' -f2
exit 0
