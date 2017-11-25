#!/bin/sh

source ./common.sh

# What are we grepping for
what=$1

# How many times should it be there
num=$2

# We assume that we read output from stdin
count=$(grep "$what" | wc -l)

if test $count -ne $num; then
    die "expected to find $num instances of '$what', found $count instead"
fi
