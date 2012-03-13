#!/bin/bash
#
#	$Id$

. ./functions.sh
header "Test grdimage for making Hovmuller plots"

gmtset TIME_EPOCH 2000-01-01T00 TIME_UNIT y
gmtset FORMAT_DATE_MAP o FORMAT_TIME_PRIMARY_MAP c FONT_ANNOT_PRIMARY 10p FONT_ANNOT_SECONDARY 12p
gmtset MAP_ANNOT_ORTHO ""

makecpt -Crainbow -T-1/1/0.05 > tmp.cpt
awk 'BEGIN{pi=3.1415;for (y=0; y<=3; y+=0.0833333) {for (x=-180; x<=180; x+=10) {print x,y,sin(2*pi*y)*sin(pi/180*x)}}}' /dev/null | \
	xyz2grd -R-180/180/0/3 -f0x,1t -I10/0.0833333 -Gtmp.nc
grdimage tmp.nc -Ctmp.cpt -JX12c/12cT -B30f10/1O -Bs/1Y -E100 > $ps

pscmp
