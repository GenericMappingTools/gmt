#!/bin/bash
#	$Id$
#
# Test gmt grdlandmask for proper wrapping.

ps=mask.ps

echo "-10000 white +10000 white" > tt.cpt
echo "N black" >> tt.cpt
# Must split a 5x5 degree shore bin across L and R boundary
gmt grdlandmask -Gtt.i2=bs -I60m -R-3/357/-90/90 -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
gmt grdimage tt.i2=bs -Jx0.017id -Bx30g180 -By30g90 -BWSnE -Ctt.cpt -Xc -Y0.75i -P -K > $ps
#
gmt grdlandmask -Gtt.i2=bs -I60m -Rg -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
gmt grdimage tt.i2=bs -J -Bx30g180 -By30g90 -BWsnE -Ctt.cpt -Y3.25i -O -K >> $ps
#
gmt grdlandmask -Gtt.i2=bs -I60m -Rd -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
gmt grdimage tt.i2=bs -J -Bx30g180 -By30g90 -BWsnE -Ctt.cpt -Y3.25i -O >> $ps

