#!/bin/sh

# This shell runs the tests with valgrind.

# $Id$

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='nctest tst_rename'

# These don't work yet: tst_fills tst_xplatform2 tst_interops6 tst_strings 

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
