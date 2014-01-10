#!/bin/bash
#
#	$Id$

ps=closed.ps

# Make a grid with closed contours at N pole, one crossing the periodic boundary, and one safely in middle
gmt set PROJ_ELLIPSOID Sphere
gmt grdmath -Rg -I1 0 0 SDIST KM2DEG 35 DIV 2 POW NEG EXP 0 90 SDIST KM2DEG 50 DIV 2 POW NEG EXP ADD 70 0 SDIST KM2DEG 35 DIV 2 POW NEG EXP ADD 11 MUL = tmp.nc
contour="gmt grdcontour -A2 -C1 -L8.5/10.5 -Gd4 tmp.nc -Bxa60g30 -By30g30 -BWS -T:LH -Wa1p,red -Wc1p,blue"
$contour -JN180/7i -P -K > $ps
$contour -JG30/35/5i -O -Y4i -X1i >> $ps

