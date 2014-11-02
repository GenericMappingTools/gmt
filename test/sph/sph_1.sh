#!/bin/bash
#
#       $Id$

ps=sph_1.ps
mars370=${src}/../../doc/examples/ex36/mars370.txt

gmt makecpt -Crainbow -T-7000/15000/1000 -Z > tt.cpt
gmt sphinterpolate ${mars370} -Rg -I1 -Q0 -Gtt.nc
gmt grdimage tt.nc -JH0/4.5i -B30g30 -B+t"-Q0" -Ctt.cpt -X0.7i -Y5.5i -K --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p > $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Q1 -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Q1" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Q2 -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Q2" -Ctt.cpt -X-5.1i -Y-5i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Q3 -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Q3" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt psxy -Rg -J -O -K ${mars370} -Sc0.05i -G0 -B30g30 -X-2.55i -Y2.5i >> $ps
gmt psxy -Rg -J -O -T >> $ps

