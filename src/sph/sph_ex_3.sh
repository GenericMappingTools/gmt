#!/bin/sh
#	$Id$
# Example of computing distances with sphdistance
ps=`basename $0 '.sh'`.ps
# Get the crude GSHHS data, select GMT format, and decimate to ~20%:
# gshhs $GMTHOME/src/coast/gshhs/gshhs_c.b | awk '{if ($1 == ">" || NR%5 == 0) print $0}' > gshhs_c.txt
# Get Voronoi polygons
sphtriangulate gshhs_c.txt -Qv -D > $$.pol
# Compute distances in km
sphdistance -Rg -I1 -Q$$.pol -G$$.nc -Lk
# Make a basic contour plot and overlay voronoi polygons and coastlines
grdcontour $$.nc -JG-140/30/7i -P -B30g30:"Distances from GSHHS crude": -K -C500 -A1000 -X0.75i -Y2i > $ps
psxy -R -J -O -K $$.pol -W0.25p,red >> $ps
pscoast -R -J -O -K -W1p -Glightgray -A0/1/1 >> $ps
psxy -Rg -J -O -T >> $ps
gv $ps &
rm -f *$$*
