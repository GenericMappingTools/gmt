#!/bin/bash
#	$Id$
#
# Basic stitching of Cartesian and geographic data

header "Test gmtstitch for assembling segments to polygon"

# Make a polygon file
gmtmath -T0/360/5 T -C0 COSD -C1 SIND -Ca T 5 MUL COSD 3 ADD MUL = t.txt
split -l 11 t.txt piece
psxy t.txt -R-5/5/-5/5 -JX3i -P -W1p -Gorange -B2g1WSne -Y5i -K > $ps
psxy piece?? -R -J -O -W1p -B2g1WSne -X3.5i -K >> $ps
gmtstitch piece?? -T0.6 | psxy -R -J -O -W1p -Gorange -B2g1WSne -X-3.5i -Y-3.5i -K >> $ps
gmtstitch piece?? -T60k -fg | psxy -R -JM3i -O -W1p -Gred -B2g1WSne -X3.5i -K >> $ps
psxy -R -J -O -T >> $ps

pscmp
