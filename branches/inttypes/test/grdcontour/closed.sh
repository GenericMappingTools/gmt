#!/bin/bash
#
#	$Id$

ps=closed.ps

# Make a grid with closed contours at N pole, one crossing the periodic boundary, and one safely in middle
grdmath -Rg -I1 0 0 SDIST 35 DIV 2 POW NEG EXP 0 90 SDIST 50 DIV 2 POW NEG EXP ADD 70 0 SDIST 35 DIV 2 POW NEG EXP ADD 11 MUL = tmp.nc
grdcontour="grdcontour -A2 -C1 -L8.5/10.5 -Gd4 tmp.nc -Ba60g30/30g30WS -T:LH -Wa1p,red -Wc1p,blue"
$grdcontour -JN180/7i -P -K > $ps
$grdcontour -JG30/35/5i -O -Y4i -X1i >> $ps

