#!/bin/sh
# This shell script runs extra tests ncdump for netcdf-4
# $Id$

set -e
echo ""
echo "*** Running extra netcdf-4 tests."

echo "*** dumping tst_string_data.nc to tst_string_data.cdl..."
./ncdump tst_string_data.nc > tst_string_data.cdl
echo "*** comparing tst_string_data.cdl with ref_tst_string_data.cdl..."
diff tst_string_data.cdl $srcdir/ref_tst_string_data.cdl

#echo '*** testing non-coordinate variable of same name as dimension...'
#../ncgen/ncgen -v4 -b -o tst_noncoord.nc $srcdir/ref_tst_noncoord.cdl

echo '*** testing reference file ref_tst_compounds2.nc...'
./ncdump $srcdir/ref_tst_compounds2.nc > tst_compounds2.cdl
diff tst_compounds2.cdl $srcdir/ref_tst_compounds2.cdl

echo '*** testing reference file ref_tst_compounds3.nc...'
./ncdump $srcdir/ref_tst_compounds3.nc > tst_compounds3.cdl
diff tst_compounds3.cdl $srcdir/ref_tst_compounds3.cdl

echo '*** testing reference file ref_tst_compounds4.nc...'
./ncdump $srcdir/ref_tst_compounds4.nc > tst_compounds4.cdl
diff tst_compounds4.cdl $srcdir/ref_tst_compounds4.cdl

echo
echo "*** All ncgen and ncdump extra test output for netCDF-4 format passed!"
exit 0
