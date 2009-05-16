#!/bin/sh
#	$Id: sph_ex_3.sh,v 1.4 2009-05-16 00:46:08 guru Exp $
# Example of computing distances with sphdistance
PS=`basename $0 '.sh'`.ps
# Get the crude GSHHS data, select GMT -m format, and decimate to ~20%:
# gshhs $GMTHOME/src/coast/gshhs/gshhs_c.b -m | awk '{if ($1 == ">" || NR%5 == 0) print $0}' > gshhs_c.txt
# Get Voronoi polygons
sphtriangulate gshhs_c.txt -Qv -m -D > $$.pol
# Compute distances in km
sphdistance -Rg -I1 -Q$$.pol -G$$.nc -Lk -m
# Make a basic contour plot and overlay voronoi polygons and coastlines
grdcontour $$.nc -JG-140/30/7i -P -B30g30:"Distances from GSHHS crude": -K -C500 -A1000 -X0.75i -Y2i > $PS
psxy -R -J -O -K -m $$.pol -W0.25p,red >> $PS
pscoast -R -J -O -K -W1p -Glightgray -A0/1/1 >> $PS
psxy -Rg -J -O /dev/null >> $PS
gv $PS &
rm -f *$$*
