#!/bin/sh

# This shell runs the tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='simple_xy_wr simple_xy_rd sfc_pres_temp_wr '\
'sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd '

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
