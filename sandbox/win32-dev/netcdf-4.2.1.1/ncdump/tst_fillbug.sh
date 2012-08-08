#!/bin/sh
# This shell script runs an ncdump bug test for netcdf-4
# $Id$

set -e
echo ""
echo "*** Running ncdump bug test."

echo "*** dumping tst_fillbug.nc to tst_fillbug.cdl..."
./ncdump tst_fillbug.nc > tst_fillbug.cdl
echo "*** comparing tst_fillbug.cdl with ref_tst_fillbug.cdl..."
diff tst_fillbug.cdl $srcdir/ref_tst_fillbug.cdl

echo
echo "*** All ncdump bug test output for netCDF-4 format passed!"
exit 0
