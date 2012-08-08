#!/bin/sh
# This shell script runs the ncgen tests.
# $Id$

echo "*** Testing ncgen."
set -e
echo "*** creating classic file c0.nc from c0.cdl..."
./ncgen -b -o c0.nc $srcdir/c0.cdl
echo "*** creating 64-bit offset file c0_64.nc from c0.cdl..."
./ncgen -k 64-bit-offset -b -o c0_64.nc $srcdir/c0.cdl

echo "*** Test successful!"
exit 0
