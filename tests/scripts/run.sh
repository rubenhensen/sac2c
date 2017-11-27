#!/bin/sh
source $(dirname $0)/common.sh

function namewe () {
    local filename="$1"
    echo "$filename" | cut -f 1 -d '.'
}

# Source file that we are about to run tests on.
SAC_FILE=$1

# Cut the path off.
base_fname=$(basename $SAC_FILE)

# Generate the name for the extracted makefile.
makefile=${base_fname}.mk

base_fname_we=$(namewe "$base_fname")

# Generate the Makefile form a sac test:
cpp -C $SAC_FILE \
     | grep "^// SAC_TEST|" \
     | sed -e 's/^\/\/ SAC_TEST|//g'\
           -e 's/<tab>/\t/g' \
           -e "s/<file-name>/$base_fname/g" \
           -e "s/<file-name-we>/$base_fname_we/g" \
           -e 's/<nlslash>/\\/g' > $makefile

if test $? -ne 0; then
    die "cannot parse SAC_TEST specification in $SAC_FILE"
fi

if ! make -f $makefile; then
    die "test $SAC_FILE failed"
fi

# else
#     echo "test $SAC_FILE passed"
# fi
