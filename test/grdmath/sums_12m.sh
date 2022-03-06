#!/usr/bin/env bash
# Test grdmath CUMSUM operators row and col sums in positive direction
ps=sums_12m.ps
gmt set MAP_TITLE_OFFSET 4p FONT_TITLE 12p
# Create a small grid and compute row and column sums
gmt grdmath -R0/5/0/3 -I1 X Y MUL = t.grd
gmt grdmath t.grd -1 CUMSUM = r.grd
gmt grdmath t.grd -2 CUMSUM = c.grd

# Plot the three grids
gmt grd2xyz t.grd | gmt pstext -R0/5/0/3 -JX6.5i/2.5i -P -B0g1 -B+tDATA -K -N -Y7.5i -F+f18p+jCM -Gwhite > $ps
gmt grd2xyz r.grd | gmt pstext -R -J -O -K -B0g1 -B+t"-1 CUMSUM" -Y-3.25i -N -F+f18p+jCM -Gwhite >> $ps
gmt grd2xyz c.grd | gmt pstext -R -J -O -B0g1 -B+t"-2 CUMSUM" -Y-3.25i -N -F+f18p+jCM -Gwhite >> $ps
