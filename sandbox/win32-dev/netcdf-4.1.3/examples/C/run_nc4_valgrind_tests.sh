#!/bin/sh

# This shell runs the tests with valgrind, for netCDF-4.

# $Id$

set -e
echo ""
echo "Testing netCDF-4 test programs with valgrind..."

# These are my test programs.
list='simple_nc4_wr simple_nc4_rd simple_xy_nc4_wr '\
'simple_xy_nc4_rd '

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
