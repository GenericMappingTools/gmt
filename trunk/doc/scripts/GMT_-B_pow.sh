#!/bin/sh
#	$Id: GMT_-B_pow.sh,v 1.5 2007-02-08 21:46:27 remko Exp $
#

gmtset GRID_PEN_PRIMARY thinnest,.
psbasemap -R0/100/0/0.9 -JX3p0.5/0.25 -Ba3f2g1p:"Axis Label":S -K -P > GMT_-B_pow.ps
psbasemap -R -J -Ba20f10g5:"Axis Label":S -O -Y0.85 >> GMT_-B_pow.ps
