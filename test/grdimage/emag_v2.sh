#!/usr/bin/env bash
# Test script for issue # msg6788 with flipped CDF order
# Orig plotted with -R is OK, region cut with grdcut fails
ps=emag_v2.ps

gmt makecpt -Crainbow -T-200/200/50 > col.cpt
gmt grdcut @EMAG2_V2_1x1.grd -Gtmp.nc -R-25/-10/60/67
gmt grdimage @EMAG2_V2_1x1.grd -P -JX4id -R -Ccol.cpt -K -Baf -BWSne -Xc > $ps
gmt pstext -R -J -O -K -F+cTR+f24p+jTR+tORIG -Gwhite >> $ps
gmt pscoast -Gblack -Dc -O -K -R -J >> $ps
gmt grdimage tmp.nc -J -Ccol.cpt -K -O -Y5i -Baf -BWSne >> $ps
gmt pscoast -Gblack -Dc -O -K -R -J >> $ps
gmt pstext -R -J -O -F+cTR+f24p+jTR+tCUT -Gwhite >> $ps
