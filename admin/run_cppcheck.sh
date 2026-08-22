#!/usr/bin/env bash
#
# Run cppcheck to check C source codes
#

CPPCHECK="cppcheck --error-exitcode=1 --language=c --std=c99 ${CPPCHECK_EXTRA_OPTIONS:-"--max-configs=1"}"

if [ "$#" == 0 ]; then
    echo "Usage: bash run_cppcheck.sh srcdir"
    exit 1
fi

if ! [ -x "$(command -v cppcheck)" ]; then
    echo 'Error: cppcheck is not found in your search PATH.' >&2
    exit 0
fi

SRCDIR=$1
find ${SRCDIR} -name '*.[ch]' \
    ! -name 'PSL_ISO-*.h' \
    ! -name 'PSL_ISOLatin*.h' \
    ! -name 'PSL_Standard*.h' \
    ! -name 'gmt_colornames.h' \
    ! -name 'gmt_cpt_masters.h' \
    ! -name 'gmt_datasets.h' \
    ! -name 'gmt_media_name.h' \
    ! -name 'gmt_unique.h' \
    ! -name 'mergesort.c' \
    -exec ${CPPCHECK} {} +
