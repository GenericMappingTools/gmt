#!/bin/sh
# This shell script runs the ncdump tests.
# $Id$

set -e
echo ""
echo "*** Testing ncgen and ncdump using some test CDL files."
echo "*** creating tst_small.nc from ref_tst_small.cdl..."
../ncgen/ncgen -o tst_small.nc $srcdir/ref_tst_small.cdl
echo "*** creating tst_small.cdl from tst_small.nc..."
./ncdump tst_small.nc > tst_small.cdl
diff -w tst_small.cdl $srcdir/ref_tst_small.cdl

echo "*** creating test0.nc from test0.cdl..."
../ncgen/ncgen -b $srcdir/test0.cdl
echo "*** creating test1.cdl from test0.nc..."
./ncdump -n test1 test0.nc > test1.cdl
echo "*** creating test1.nc from test1.cdl..."
../ncgen/ncgen -b test1.cdl
echo "*** creating test2.cdl from test1.nc..."
./ncdump test1.nc > test2.cdl
echo "*** checking that test1.cdl and test2.cdl are the same..."
diff -w test1.cdl test2.cdl

echo "*** All tests of ncgen and ncdump using test0.cdl passed!"
exit 0
