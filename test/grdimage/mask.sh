#!/usr/bin/env bash
#
# Test colorizing 1-bit images in grdimage

ps=mask.ps

# Created mask.nc this way, now in git
# gmt grdmath -R0/20/0/20 -I1 -r 0 1 RAND RINT = mask.nc=nb
echo "0	white" > t.cpt
echo "1 black" >> t.cpt
gmt grdimage mask.nc -Ct.cpt -JX2.5i -Xc -Gorange+b -Baf -BWSne -K -P > $ps
gmt grdimage mask.nc -Ct.cpt -J -Y3i -Baf -BWSne -O -K >> $ps
gmt grdimage mask.nc -Ct.cpt -J -Y3i -Gorange+f -Baf -BWSne -O >> $ps
