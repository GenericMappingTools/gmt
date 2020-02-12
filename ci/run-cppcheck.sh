#!/usr/bin/env bash
#
# Check C source codes
#
# For scheduled CI jobs, check all C source codes;
# For PRs, check changed files only.
#

CPPCHECK="cppcheck --error-exitcode=1 --std=c99 --force --quiet"

if [ "$BUILD_REASON" == "Schedule" ]; then
    # Check all C codes for scheduled jobs
    find . -name '*.[ch]' \
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
else
    # Check changed files only for PRs
    target_branch=${SYSTEM_PULLREQUEST_TARGETBRANCH:-master}
    changed_files=$(git diff --name-only origin/${target_branch}..HEAD | grep -E '.*\.(c|h)$')
    if [ $? == 0 ]; then
        ${CPPCHECK} $changed_files
    else
        echo "No C files changed! Skip cppcheck."
    fi
fi
