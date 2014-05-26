#!/bin/bash
#	$Id$
# Place 3 lines with text along the straight lines and rounded boxes
ps=quotedstraight.ps
echo "> -L\"The first curve\"" > data.txt
gmt math -T0/180/45 T 180 DIV = >> data.txt
echo "> -L\"The second curve\"" >> data.txt
gmt math -T0/180/45 T 180 DIV 2 ADD = >> data.txt
echo "> -L\"The third curve\"" >> data.txt
gmt math -T0/180/45 T 180 DIV 4 ADD = >> data.txt
gmt psxy -R-5/185/-1/6 -JX6i/9i -P -K -W1p,red -Sqn2:+f12p+Lh+o+e data.txt --PS_COMMENTS=true > $ps
gmt psbasemap -R -J -O -K -B0 -B+gyellow --PS_COMMENTS=true >> $ps
gmt psclip -R -J -O -C -B+t"Clipped text with yellow on top" >> $ps
