#!/usr/bin/env bash
#
# Script to run the C naming convention enforcer to determine if
# some functions are misnamed or misplaced.  All results are written
# to /tmp/gmt unless -o is used.
#
#   admin/src_convention_check.sh [-e] [-f] [-o] [-v] [-w]
#

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi

# Set up the temp and possibly output directory
rm -rf /tmp/gmt
mkdir -p /tmp/gmt

# Create 3 include files based on current gmt function status

# 1. Create the list of current API prototype (GMT_*) functions from gmt.h
# [The tr command is there to protect against bad declarations like int *get_integer. (There should be a space after the *)]
egrep '^EXTERN_MSC' src/gmt.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s\",\n", $1}' > /tmp/gmt/api.h
# 2. Create the list of current prototype (gmt_*) functions from gmt_prototypes.h
egrep '^EXTERN_MSC' src/gmt_prototypes.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s\",\n", $1}' > /tmp/gmt/prototypes.h
# 3. Create the list of current prototype (gmtlib_*) functions from gmt_internals.h
egrep '^EXTERN_MSC' src/gmt_internals.h | awk -F'(' '{print $1}' | awk '{print $NF}' | tr '*' ' ' | awk '{printf "\t\"%s\",\n", $1}' > /tmp/gmt/internals.h
# Create list ofboth classic and modern mode module functions
gmt --show-modules > /tmp/gmt/all_modules.lis
gmt --show-classic >> /tmp/gmt/all_modules.lis
echo "gmtread" >> /tmp/gmt/all_modules.lis
echo "gmtwrite" >> /tmp/gmt/all_modules.lis
egrep '^EXTERN_MSC' src/gmt_compat.c | awk -F'(' '{print $1}' | awk '{print substr($NF,5)}' >> /tmp/gmt/all_modules.lis

sort -u /tmp/gmt/all_modules.lis | awk '{printf "\t\"%s\",\n", $1}' > /tmp/gmt/modules.h
gcc admin/src_convention_check.c -o /tmp/src_convention_check

find src -name '*.c' | egrep -v 'triangle.c|mergesort.c|test|example|demo|kiss|ssrfpack|stripack|s_rint|qsort.c|cm4_functions' > /tmp/gmt/c_codes.lis
/tmp/src_convention_check $* $(cat /tmp/gmt/c_codes.lis)
