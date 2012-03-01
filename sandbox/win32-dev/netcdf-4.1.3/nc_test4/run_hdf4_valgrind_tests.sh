#!/bin/sh

# This shell runs the HDF4 tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='tst_interops2 '

for tst in $list; do
    echo ""
    echo "Memory testing with $tst:"
    valgrind -q --error-exitcode=2 --leak-check=full ./$tst
done

echo "SUCCESS!!!"

exit 0
