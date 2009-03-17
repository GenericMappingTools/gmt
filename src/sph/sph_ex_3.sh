#!/bin/sh
#	$Id: sph_ex_3.sh,v 1.1 2009-03-17 00:18:42 myself Exp $
# Example of computing distances with sphdistance
PS=`basename $0 '.sh'`.ps
PDF=`basename $0 '.sh'`.pdf
# Get the crude GSHHS data and select GMT -M format:
# gshhs $GMTHOME/src/coast/gshhs/gshhs_c.b -M > gshhs_c.txt
# Get Voronoi polygons
sphtriangulate gshhs_f.txt -Qv -M -D > $$.pol
# Compute distances in km
sphdistance -Rg -I1 -Q$$.pol -G$$.nc -Lk -M
# Make a basic contour plot and overlay voronoi polygons and coastlines
grdcontour $$.nc -JG-140/30/7i -P -B30g30:"Distances from GSHHS crude": -K -C500 -A1000 -X0.75i -Y2i > $PS
psxy -R -J -O -K -M $$.pol -W0.25p,red >> $PS
pscoast -R -J -O -K -W1p -Glightgray -A0/1/1 >> $PS
psxy -Rg -J -O /dev/null >> $PS
ps2raster -Tf $PS
open $PDF
rm -f $PS *$$*
