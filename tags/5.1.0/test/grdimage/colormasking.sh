#!/bin/bash
#
#	$Id$

ps=colormasking.ps

gmt xyz2grd -R-0.5/2.5/-0.5/2.5 -I1 -r -Gt.nc <<%
0 0 0.0
0 1 0.2
0 2 0.4
1 0 1.0
1 1 NaN
1 2 1.4
2 0 2.0
2 1 2.2
2 2 2.4
%
gmt makecpt -Crainbow -T-0.1/2.5/0.2 > t.cpt
gmt grdimage t.nc -JX1c -Ct.cpt -K -P -X8c -Y8c > $ps
gmt grdimage t.nc -JX3c -Ct.cpt -K -O -X-1c -Y-1c -Q >> $ps
gmt grdimage t.nc -JX9c -Ct.cpt -K -O -X-3c -Y-3c -Q >> $ps
gmt psxy -R -J -Gp50/10:FwhiteB- -O >> $ps <<%
1 0
2 0
2 1
1 1
%

