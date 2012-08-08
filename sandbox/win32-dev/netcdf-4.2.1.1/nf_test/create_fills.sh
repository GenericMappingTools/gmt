#!/bin/sh
# This shell script which creates the fill.nc file from fill.cdl.
# $Id$

echo
echo "*** Testing creating file with fill values."
set -e
../ncgen/ncgen -b $srcdir/fills.cdl
echo "*** SUCCESS!"
exit 0
