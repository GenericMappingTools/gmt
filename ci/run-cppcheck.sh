#!/usr/bin/env bash
#
# Check C source codes
#
# For scheduled CI jobs, check all C source codes;
# For PRs, check changed files only.
#

CPPCHECK="cppcheck --error-exitcode=1 --force"

if [ "$BUILD_REASON" == "Schedule" ]; then
    # Check all C codes for scheduled jobs
    find . -name '*.[ch]' -exec ${CPPCHECK} {} +
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
