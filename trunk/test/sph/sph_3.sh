#!/bin/bash
#
#       $Id: sph_3.sh,v 1.2 2011-06-18 04:07:36 guru Exp $

. ../functions.sh
header "Testing sphtriangulate and sphdistance"

ps=sph_3.ps

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
rm -f *$$*

pscmp
