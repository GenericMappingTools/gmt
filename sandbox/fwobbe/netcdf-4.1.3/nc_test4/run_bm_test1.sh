#!/bin/sh

# This shell just tests the bm_file program by running it a few times
# on a simple test file. Then it uses ncdum to check that the output
# is what it should be.

# $Id$

set -e
echo ""

for type_name in floats ints shorts
do
    echo "*** Running benchmarking program bm_file for simple ${type_name} test files, 1D to 6D..."
    header="-h"
    for i in 1 2 3 4 5 6
    do
	test $i = 1 && chunksizes="100000"
	test $i = 2 && chunksizes="316:316"
	test $i = 3 && chunksizes="46:46:46"
	test $i = 4 && chunksizes="17:17:17:17" && s="-s 1"
	test $i = 5 && chunksizes="10:10:10:10:10" && s="-s 1"
	test $i = 6 && chunksizes="6:6:6:6:6:6" && s="-s 1"
	./bm_file -d ${header} ${s} -f 4 -o ${type_name}2_${i}D_3.nc -c 0:0:0:${chunksizes} tst_${type_name}2_${i}D.nc 
	../ncdump/ncdump tst_${type_name}2_${i}D.nc > tst_${type_name}2_${i}D.cdl
	../ncdump/ncdump -n tst_${type_name}2_${i}D ${type_name}2_${i}D_3.nc > ${type_name}2_${i}D.cdl
	diff tst_${type_name}2_${i}D.cdl ${type_name}2_${i}D.cdl
	header=
    done
    echo '*** SUCCESS!!!'
done

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, no compression..."
./bm_file -d -h -f 3 -o floats_1D_3.nc -c 0:-1:0:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, with compression..."
./bm_file -d -h -f 3 -o floats_1D_3.nc -c 0:1:0:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, with more compression..."
./bm_file -d -h -f 3 -o floats_1D_3.nc -c 0:9:1:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, with endianness set to native..."
./bm_file -e 0 -d -h -f 3 -o floats_1D_3.nc -c 0:9:1:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, with endianness set to little..."
./bm_file -e 1 -d -h -f 3 -o floats_1D_3.nc -c 0:9:1:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

echo ""
echo "*** Testing the benchmarking program bm_file for simple float file, with endianness set to big..."
./bm_file -e 2 -d -h -f 3 -o floats_1D_3.nc -c 0:9:1:10000 tst_floats_1D.nc
../ncdump/ncdump tst_floats_1D.nc > tst_floats_1D.cdl
../ncdump/ncdump -n tst_floats_1D floats_1D_3.nc > floats_1D.cdl
diff tst_floats_1D.cdl floats_1D.cdl
echo '*** SUCCESS!!!'

exit 0