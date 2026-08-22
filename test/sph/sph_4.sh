#!/usr/bin/env bash

ps=sph_4.ps

# Use the locations of a global hotspot file
gmt sphtriangulate @hotspots.txt -Qd -T > tt.arcs
# Make a basic contour plot and overlay voronoi polygons and coastlines
gmt pscoast -Rg -JG-120/-30/7i -P -B30g30 -B+t"Delaunay triangles from hotspots" -Glightgray -K -X0.75i -Y2i > $ps
gmt psxy -R -J -O -K tt.arcs -W1p >> $ps
gmt psxy -R -J -O -W0.25p -SE-250 -Gred @hotspots.txt >> $ps
