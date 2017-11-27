#!/bin/sh

source $(dirname $0)/common.sh

# We check the return status of this binary:
binary=$1

# The return status should be this value:
ret=$2

./$binary
if test $? -ne $ret; then
    die "'$binary' expected to return $ret as exit code"
fi
