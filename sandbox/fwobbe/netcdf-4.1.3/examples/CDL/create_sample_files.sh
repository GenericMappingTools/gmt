#!/bin/sh
# This shell script creates the netCDF example files from CDL scipts.
# $Id$

set -e
echo ""
echo "*** Creating example data files from CDL scripts."
echo "*** creating simple_xy.nc..."
../../ncgen/ncgen -o simple_xy.nc $srcdir/simple_xy.cdl

echo "*** checking sfc_pres_temp.nc..."
../../ncgen/ncgen -o sfc_pres_temp.nc $srcdir/sfc_pres_temp.cdl

echo "*** checking pres_temp_4D.nc..."
../../ncgen/ncgen -o pres_temp_4D.nc $srcdir/pres_temp_4D.cdl

echo "*** All example creations worked!"
exit 0
