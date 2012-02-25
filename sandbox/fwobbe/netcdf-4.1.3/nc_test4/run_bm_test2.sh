#!/bin/sh

# This shell tests non-sequential reads for bm_file.c

# $Id$

set -e
echo ""

echo "*** Testing the benchmarking program bm_file using start/count/inc..."
./bm_file -h -f 3 -o tst_simple_3.nc -c 0:-1:0:10  -t 0:0 -u 0:1 -r 0:1 tst_simple.nc
../ncdump/ncdump tst_simple.nc > tst_simple.cdl
../ncdump/ncdump -n tst_simple tst_simple_3.nc > tst_simple_3.cdl
diff tst_simple.cdl tst_simple_3.cdl
echo '*** SUCCESS!!!'

echo "*** Testing the benchmarking program bm_file using start/count/inc with negative increment..."
./bm_file -h -f 3 -o tst_simple_3.nc -c 0:-1:0:10  -t 0:9 -u 0:1 -r 0:-1 tst_simple.nc
../ncdump/ncdump tst_simple.nc > tst_simple.cdl
../ncdump/ncdump -n tst_simple tst_simple_3.nc > tst_simple_3.cdl
diff tst_simple.cdl tst_simple_3.cdl
echo '*** SUCCESS!!!'

echo "*** Testing the benchmarking program bm_file using start/count/inc with count > 1..."
./bm_file -h -f 3 -o tst_simple_3.nc -c 0:-1:0:10  -t 0:0 -u 0:2 -r 0:2 tst_simple.nc
../ncdump/ncdump tst_simple.nc > tst_simple.cdl
../ncdump/ncdump -n tst_simple tst_simple_3.nc > tst_simple_3.cdl
diff tst_simple.cdl tst_simple_3.cdl
echo '*** SUCCESS!!!'

echo "*** Testing the benchmarking program bm_file using start/count/inc with uneven count..."
./bm_file -h -f 3 -o tst_simple_3.nc -c 0:-1:0:10  -t 0:0 -u 0:4 -r 0:4 tst_simple.nc
../ncdump/ncdump tst_simple.nc > tst_simple.cdl
../ncdump/ncdump -n tst_simple tst_simple_3.nc > tst_simple_3.cdl
diff tst_simple.cdl tst_simple_3.cdl
echo '*** SUCCESS!!!'

exit 0