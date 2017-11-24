#!/bin/sh

SAC_FILE=$1

namewe=$(basename $SAC_FILE)
makefile=${namewe}.mk

# Generate the Makefile form a sac test:
cpp -C $SAC_FILE \
     | grep "^// SAC_TEST:" \
     | sed -e 's/^\/\/ SAC_TEST://g'\
           -e 's/<tab>/\t/g' \
           -e 's/<nlslash>/\\/g' > $makefile

if ! make -f $makefile; then
    echo "test $SAC_FILE failed"
    exit 1
fi

# else
#     echo "test $SAC_FILE passed"
# fi
