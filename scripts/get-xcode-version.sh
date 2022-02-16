#!/usr/bin/env bash

# this script gets the version of XCode currently 'activated'
# in the environment using `xcodebuild`.

if [ ! -x "/usr/bin/xcodebuild" ]; then
    echo "ERROR: XCode doesn't seem to be installed!" >&2
    exit 1
fi

version=$(/usr/bin/xcodebuild -version 2>/dev/null)
if [ $? -ne 0 ]; then
    echo "ERROR: \`xcodebuild' commandline tool has failed!" >&2
    exit 2
fi

# the output from -version is `XCode VERSION`, we return only the second field
echo "${version}" | /usr/bin/grep -E "[0-9]+\.[0-9]+\.[0-9]+" | /usr/bin/cut -d' ' -f2
exit 0
