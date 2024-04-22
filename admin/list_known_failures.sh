#!/usr/bin/env bash
#
# Report the list of known failures.
#

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi

known_failures=$(grep -rl "GMT_KNOWN_FAILURE" test/**/*.sh doc/**/*.sh)
if [ -n "$known_failures" ]; then
    echo "The following tests are marked as GMT_KNOWN_FAILURE and need review:"
    echo ""
    while IFS= read -r line; do
        echo "- $line"
    done <<< $known_failures
fi
