#!/usr/bin/env bash
# Test slanted Cartesian x-axis annotations in the presence of label and title
ps=slantlabel.ps
gmt psbasemap -R0/5000/0/20 -JX16c/6c -Bxaf+a60+l"MY VERY LONG LABEL" -Byaf -B+t"THIS IS MY TITLE" -K -P -Y3c > $ps
gmt psbasemap -R -J -Bxaf+a-25+l"MY VERY LONG LABEL" -Byaf -B+t"THIS IS MY TITLE" -O -Y14c >> $ps
