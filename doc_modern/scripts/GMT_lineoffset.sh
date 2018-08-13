#!/bin/bash
#
ps=GMT_lineoffset.ps
gmt math -T10/30/1 T 20 SUB 10 DIV 2 POW 41.5 ADD = line.txt

gmt psxy line.txt -R8/32/40/44 -JM5i -P -Wfaint,red -K -Bxaf -Bya2f1 -BWSne --MAP_FRAME_TYPE=plain > $ps
gmt psxy line.txt -R -J -W2p+o1c/500k -O -K >> $ps
gmt pstext -R -J -O -K -F+f10p+jCM+a << EOF >> $ps
11.0 42.6 -11.5 1 cm
27.1 42.3 +9.5 500 km
EOF
gmt psxy -R -J -O -T >> $ps
