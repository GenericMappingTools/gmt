#!/bin/bash
#	$Id: stitch.sh,v 1.2 2011-06-29 02:28:24 guru Exp $
#
# Basic stitching of Cartesian and geographic data

. ../functions.sh
header "Test gmtstitch for assembling segments to polygon"

ps=stitch.ps

# Make a polygon file
gmtmath -T0/360/5 T -C0 COSD -C1 SIND -Ca T 5 MUL COSD 3 ADD MUL = t.txt
split -l 10 t.txt piece
psxy t.txt -R-5/5/-5/5 -JX3i -P -W1p -Gorange -B2g1WSne -Y5i -K > $ps
psxy piece?? -R -J -O -W1p -B2g1WSne -X3.5i -K >> $ps
gmtstitch piece?? -T0.1 | psxy -R -J -O -W1p -Gorange -B2g1WSne -X-3.5i -Y-3.5i -K >> $ps
gmtstitch piece?? -T25k -fg | psxy -R -JM3i -O -W1p -Gred -B2g1WSne -X3.5i -K >> $ps
psxy -R -J -O -T >> $ps
#rm -f t.txt piece??
pscmp
