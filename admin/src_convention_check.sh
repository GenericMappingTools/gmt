#!/usr/bin/env bash
#
# Script to run the C naming convention enforcerer to determine if
# some functions are misnamed or misplaced.
#
#   admin/src_convention_check.sh > logfile
#

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi

gcc admin/src_convention_check.c -o /tmp/src_convention_check

/tmp/src_convention_check `find . -name '*.c'`
