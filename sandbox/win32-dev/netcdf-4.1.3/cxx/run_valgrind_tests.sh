#!/bin/sh

# This shell runs the tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list=""
# If we are running with netcdf4, then add tst_atts
if test "x$USE_NETCDF4" = "x1" ; then
list="$list tst_many_writes"
fi

for tst in $list; do
    echo ""
    echo "Memory testing with $tst:"
    valgrind -q --error-exitcode=2 --leak-check=full ./$tst
done

echo "SUCCESS!!!"

exit 0
