#!/bin/sh
#	$Id: GMT_-B_pow.sh,v 1.2 2003-04-12 00:40:42 pwessel Exp $
#

gmtset GRID_PEN 0.25top
psbasemap -R0/100/0/0.9 -JX3p0.5/0.25 -Ba3f2g1p:"Axis Label":S -K -P > GMT_-B_pow.ps
psbasemap -R -JX -Ba20f10g5:"Axis Label":S -O -Y0.85 >> GMT_-B_pow.ps
gmtset GRID_PEN 0.25p
