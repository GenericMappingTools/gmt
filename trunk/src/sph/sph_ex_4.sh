#!/bin/sh
#	$Id$
# Example of computing Delauney triangles with gmt sphgmt triangulate
ps=`basename $0 '.sh'`.ps
# Use the locations of a global hotspot file
gmt sphgmt triangulate hotspots.d -Qd -T > $$.arcs
# Make a basic contour plot and overlay voronoi polygons and coastlines
gmt pscoast -Rg -JG-120/-30/7i -P -B30g30:"Delaunay triangles from hotspots": -Glightgray -K -X0.75i -Y2i > $ps
gmt psxy -R -J -O -K $$.arcs -W1p >> $ps
gmt psxy -R -J -O -K -W0.25p -Sc0.1i -Gred hotspots.d >> $ps
gmt psxy -R -J -O -T >> $ps
gv $ps &
rm -f *$$*
