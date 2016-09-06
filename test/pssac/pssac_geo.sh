#!/bin/bash
#	$Id$
#
# Description:

J=M11c
R=-120/-40/30/70
Bx=x10
By=y5
PS=pssac_geo.ps
SACFILEs="${src:-.}"/*.z

gmt psxy -J$J -R$R -T -K > $PS
# left
gmt pssac $SACFILEs -J$J -R$R -BWSen -B$Bx -B$By -M0.5i -S800i -G+gblue -K -O >> $PS
gmt psxy station.list -J$J -R$R -St0.4c -Gblack -i1,2 -K -O >> $PS

# right
gmt pssac $SACFILEs -J$J -R$R -BWSen -B$Bx -B$By -M0.5i -S2032c -Q -G+gblue -K -O -X13c >> $PS
gmt psxy station.list -J$J -R$R -St0.4c -Gblack -i1,2 -K -O >> $PS

gmt psxy -J$J -R$R -T -O >> $PS
