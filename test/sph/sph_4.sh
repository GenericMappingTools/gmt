#!/bin/bash
#
#       $Id$

. ../functions.sh
header "Testing sphtriangulate II"

ps=sph_4.ps

# Use the locations of a global hotspot file
sphtriangulate hotspots.d -Qd -T > $$.arcs
# Make a basic contour plot and overlay voronoi polygons and coastlines
pscoast -Rg -JG-120/-30/7i -P -B30g30:"Delaunay triangles from hotspots": -Glightgray -K -X0.75i -Y2i > $ps
psxy -R -J -O -K $$.arcs -W1p >> $ps
psxy -R -J -O -K -W0.25p -Sc0.1i -Gred hotspots.d >> $ps
psxy -R -J -O -T >> $ps
rm -f *$$*

pscmp
