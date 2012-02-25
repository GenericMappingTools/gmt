#!/bin/sh

# This shell runs the tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list="t_type tst_camrun tst_vl tst_v2 tst_vars2 \
tst_atts2 tst_files tst_atts"

for tst in $list; do
    echo ""
    echo "Memory testing with $tst:"
    valgrind -q --error-exitcode=2 --leak-check=full ./$tst
done

echo "SUCCESS!!!"

exit 0
