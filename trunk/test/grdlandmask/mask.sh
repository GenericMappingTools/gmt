#!/bin/bash
#	$Id$
#
# Test grdlandmask for proper wrapping.

header "Test grdlandmask when wrapping over 0-360"

echo "-10000 white +10000 white" > tt.cpt
echo "N black" >> tt.cpt
# Must split a 5x5 degree shore bin across L and R boundary
grdlandmask -Gtt.i2=bs -I60m -R-3/357/-90/90 -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage tt.i2=bs -Jx0.017id -B30g180/30g90WSnE -Ctt.cpt -Xc -Y0.75i -P -K > $ps
#
grdlandmask -Gtt.i2=bs -I60m -Rg -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage tt.i2=bs -J -B30g180/30g90WsnE -Ctt.cpt -Y3.25i -O -K >> $ps
#
grdlandmask -Gtt.i2=bs -I60m -Rd -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage tt.i2=bs -J -B30g180/30g90WsnE -Ctt.cpt -Y3.25i -O >> $ps

pscmp
