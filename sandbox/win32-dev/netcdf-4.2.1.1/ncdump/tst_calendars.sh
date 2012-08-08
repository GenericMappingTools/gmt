#!/bin/sh
# This shell script tests ncdump -t option for CF calendar attributes
# $Id $

set -e
echo ""
echo "*** Testing ncdump -t output for times with CF calendar attribute"
echo "*** creating netcdf file tst_calendars.nc from tst_calendars.cdl..."
../ncgen/ncgen -b -o tst_calendars.nc $srcdir/tst_calendars.cdl
echo "*** creating tst_times.cdl from tst_calendars.nc with ncdump -t ..."
./ncdump -n tst_times -t tst_calendars.nc > tst_times.cdl
echo "*** comparing tst_times.cdl with ref_times.cdl..."
diff -b tst_times.cdl $srcdir/ref_times.cdl
echo "*** All ncdump test output for -t option with CF calendar atts passed!"
exit 0
