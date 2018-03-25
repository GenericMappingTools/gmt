#!/bin/bash
# Test script for issue # msg6788 with flipped CDF order
# Orig plotted with -R is OK, region cut with grdcut fails
ps=emag_v2.ps

gmt makecpt -Crainbow -T-200/200/50 > col.cpt
gmt grdcut @EMAG2_V2.grd -Gtmp.nc -R-25/-10/60/67
gmt grdimage @EMAG2_V2.grd -P -JM4i -R -Ccol.cpt -K -Baf -BWSne -Xc > $ps
echo ORIG | gmt pstext -R -J -O -K -F+cTR+f24p+jTR -Gwhite >> $ps
gmt pscoast -Gblack -Dc -O -K -R -J >> $ps
gmt grdimage tmp.nc -J -Ccol.cpt -K -O -Y5i -Baf -BWSne >> $ps
gmt pscoast -Gblack -Dc -O -K -R -J >> $ps
echo CUT | gmt pstext -R -J -O -F+cTR+f24p+jTR -Gwhite >> $ps
