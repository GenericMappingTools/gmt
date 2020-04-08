#!/usr/bin/env bash
#
# Script to run the C naming convention enforcer to determine if
# some functions are misnamed or misplaced.  All results are written
# to /tmp/gmt
#
#   admin/src_convention_check.sh [-e] [-f]
#

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi

# Set up the output directory
rm -rf /tmp/gmt
mkdir -p /tmp/gmt

# Create include files based on current gmt status
# Create the list of current API prototype (GMT_*) functions from gmt.h
grep EXTERN_MSC src/gmt.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s.c\",\n", $1}' > /tmp/gmt/api.h
# Create the list of current prototype (gmt_*) functions from gmt_prototypess.h
grep EXTERN_MSC src/gmt_prototypes.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s.c\",\n", $1}' > /tmp/gmt/prototypes.h
# Create the list of current prototype (gmtlib_*) functions from gmt_internals.h
grep EXTERN_MSC src/gmt_internals.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s.c\",\n", $1}' > /tmp/gmt/internals.h
# Create list of module functions
gmt --show-classic | awk '{printf "\t\"%s.c\",\n", $1}' > /tmp/gmt/modules.h
gcc admin/src_convention_check.c -o /tmp/src_convention_check

find src -name '*.c' | egrep -v 'triangle.c|mergesort.c|test|example|demo|kiss|ssrfpack|stripack|s_rint|qsort.c' > /tmp/gmt/c_codes.lis
/tmp/src_convention_check $* `cat /tmp/gmt/c_codes.lis`
