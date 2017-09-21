#!/bin/bash
#	$Id$
#
# Description:

J=M11c
R=-120/-40/30/70
Bx=x10
By=y5
ps=pssac_geo.ps
SACFILEs="${src:-.}"/*.z

gmt psxy -J$J -R$R -T -K > $ps
# left
gmt pssac $SACFILEs -J -R -BWSen -B$Bx -B$By -M0.5i -S800i -G+gblue -K -O >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -K -O >> $ps

# right
gmt pssac station.list -J -R -BWSen -B$Bx -B$By -M0.5i -S2032c -Q -G+gblue -K -O -X13c >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -K -O >> $ps

gmt psxy -J -R -T -O >> $ps
