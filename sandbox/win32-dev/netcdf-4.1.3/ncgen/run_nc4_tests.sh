#!/bin/sh
# This shell script runs the ncdump tests.
# $Id$

echo "*** Testing ncgen for netCDF-4."
set -e
echo "*** creating netCDF-4 file c0_4.nc from c0.cdl..."
./ncgen -k3 -b -o c0_4.nc $srcdir/c0.cdl
echo "*** creating netCDF-4 classic model file c0_4c.nc from c0.cdl..."
./ncgen -k4 -b -o c0_4c.nc $srcdir/c0.cdl
echo "*** creating C code for CAM file ref_camrun.cdl..."
./ncgen -lc $srcdir/ref_camrun.cdl >ref_camrun.c

echo "*** Test successful!"
exit 0
