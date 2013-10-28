#!/bin/bash
#
#       $Id$

ps=sph_3.ps

# Get the crude GSHHS data, select GMT format, and decimate to ~20%:
# gshhs $GMTHOME/src/coast/gshhs/gshhs_c.b | $AWK '{if ($1 == ">" || NR%5 == 0) print $0}' > gshhs_c.txt
# Get Voronoi polygons
gmt sphtriangulate ${src}/../../doc/examples/ex35/gshhs_c.txt -Qv -D > tt.pol
# Compute distances in km
gmt sphdistance -Rg -I1 -Qtt.pol -Gtt.nc -Lk
# Make a basic contour plot and overlay voronoi polygons and coastlines
gmt grdcontour tt.nc -JG-140/30/7i -P -B30g30 -B+t"Distances from GSHHS crude" -K -C500 -A1000 \
	-L500 -GL0/90/203/-10,175/60/170/-30,-50/30/220/-5 -X0.75i -Y2i > $ps
gmt psxy -R -J -O -K tt.pol -W0.25p,red >> $ps
gmt pscoast -R -J -O -K -W1p -Glightgray -A0/1/1 >> $ps
gmt psxy -Rg -J -O -T >> $ps
