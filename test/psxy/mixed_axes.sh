#!/bin/bash
# Check mix of geographic and Cartesian axes and point placements
# This was initially created to test issue # 520
ps=mixed_axes.ps
gmt set FORMAT_GEO_MAP dddF
echo 135E 500 | gmt psxy -R128E/152E/0/700 -JX2.5i/4i -Ba5/a100 -Sc0.2i -Gred -P -K > $ps
echo 135E 500 | gmt psxy -R -JX2.5i/-4i -Ba5/a100 -Sc0.2i -Gred -O -K -X3.75i >> $ps
echo 300 5S   | gmt psxy -R0/400/15S/30N -JX2.5i/4i -Ba100/a10 -Sc0.2i -Gblue -X-3.75i -Y5i -O -K >> $ps
echo 300 5S   | gmt psxy -R -JX-2.5i/4i -Ba100/a10 -Sc0.2i -Gblue -X3.75i -O >> $ps
