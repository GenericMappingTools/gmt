#!/bin/sh

# This shell runs the tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='t_nc tst_norm tst_names tst_misc nc_test'
# If we are running with netcdf4, then add tst_atts
if test "x$USE_NETCDF4" = "x1" ; then
list="$list tst_atts"
fi

# These don't work yet: tst_fills tst_xplatform2 tst_interops6 tst_strings 

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
