#!/bin/sh

# This shell runs the tests with valgrind.

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='tst_h_files tst_h_files2 tst_h_files4 tst_h_atts '\
'tst_h_atts3 tst_h_atts4 tst_h_vars tst_h_vars2 tst_h_vars3 tst_h_grps '\
'tst_h_compounds tst_h_compounds2 tst_h_wrt_cmp tst_h_rd_cmp tst_h_vl '\
'tst_h_opaques tst_h_strings tst_h_strings1 tst_h_strings2 tst_h_ints '\
'tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2 tst_h_dimscales3 '\
'tst_h_enums'

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
