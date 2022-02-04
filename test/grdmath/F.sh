#!/usr/bin/env bash
# DVC_TEST
ps=F.ps
gmt grdmath -R1/10/1/15 -I1 0.95 X Y FCRIT = F.nc
gmt psbasemap -R0.5/10.5/0.5/15.5 -JX6i/-9i -Bg1+0.5 -P -K -X1.5i -Y0.5i > $ps
gmt grd2xyz F.nc --FORMAT_FLOAT_OUT=%.2f > F.txt
gmt pstext -R -J -F+f12p -O -Ba1 F.txt \
	-Bx+l"Degrees of freedom for numerator" -By+l"Degrees of freedom for denomitor" -BWN+t"Critical F table (@~a@~ = 0.05)" >> $ps
