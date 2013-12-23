#!/bin/bash
#	$Id$
#
# Basic segment connection of Cartesian and geographic data

ps=connect.ps

# Make a polygon file
gmt gmtmath -T0/360/5 T -C0 COSD -C1 SIND -Ca T 5 MUL COSD 3 ADD MUL = t.txt
split -l 11 t.txt piece
gmt psxy t.txt -R-5/5/-5/5 -JX3i -P -W1p -Gorange -B2g1 -BWSne -Y5i -K > $ps
gmt psxy piece?? -R -J -O -W1p -B2g1 -BWSne -X3.5i -K >> $ps
gmt gmtconnect piece?? -T0.6 | gmt psxy -R -J -O -W1p -Gorange -B2g1 -BWSne -X-3.5i -Y-3.5i -K >> $ps
gmt gmtconnect piece?? -T60k -fg | gmt psxy -R -JM3i -O -W1p -Gred -B2g1 -BWSne -X3.5i -K >> $ps
gmt psxy -R -J -O -T >> $ps
