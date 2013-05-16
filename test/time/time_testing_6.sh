#!/bin/bash
#
#	$Id$
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 6:
# We create a grid with gmt grdmath and then make a contour map
# and associate x with days relative to the start of 2004.

ps=time_testing_6.ps

gmt grdmath -R0/90/0/90 -I1 X Y ADD = tt6.nc
gmt grdcontour tt6.nc -JX6t/6 -C5 -A10 --TIME_EPOCH=2004-01-01T --TIME_UNIT=d  -Bpxa7Rf1d -Bpy10 -Bsxa1O -P --FORMAT_DATE_MAP="-o-yy" --FONT_ANNOT_PRIMARY=9p > $ps

