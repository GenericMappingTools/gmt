#!/usr/bin/env bash
#
# Description:

J=M9c
R=-120/-40/35/70
Bx=xaf
By=ya
ps=pssac_geo.ps
SACFILEs="${src:-.}"/*.z

gmt psxy -J$J -R$R -T -K > $ps
# lower left
gmt pssac $SACFILEs -J -R -BWSen -B$Bx -B$By -M0.5i -S1000i -G+gblue -W -K -O >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -K -O >> $ps

# lower right
gmt pssac station.list -J -R -BWSen -B$Bx -B$By -M0.5i -S400c -Q -G+gblue -W -K -O -X13c >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -K -O >> $ps

# upper left
gmt pssac $SACFILEs -J -R -BWSen -B$Bx -B$By -M0.5i -Si0.001i -G+gblue -W -K -O -X-13c -Y9.5c >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -K -O >> $ps

# upper right
gmt pssac station.list -J -R -BWSen -B$Bx -B$By -M0.5i -Si0.0025c -Q -G+gblue -W -K -O -X13c >> $ps
gmt psxy station.list -J -R -St0.4c -Gblack -i1,2 -O >> $ps
