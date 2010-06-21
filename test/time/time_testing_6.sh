#!/bin/sh
#
#	$Id: time_testing_6.sh,v 1.8 2010-06-21 23:55:22 guru Exp $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 6:
# We create a grid with grdmath and then make a contour map
# and associate x with days relative to the start of 2004.

. ../functions.sh
header "Test time conversions (abs & rel time)"

ps=time_testing_6.ps

grdmath -R0/90/0/90 -I1 X Y ADD = $$.nc
grdcontour $$.nc -JX6t/6 -C5 -A10 --TIME_EPOCH=2004-01-01T --TIME_UNIT=d  -Bpa7Rf1d/10 -Bsa1O/10 -P --PLOT_DATE_FORMAT="-o-yy" --ANNOT_FONT_SIZE_PRIMARY="+9p" > $ps

rm -f $$.* .gmtcommands*

pscmp
