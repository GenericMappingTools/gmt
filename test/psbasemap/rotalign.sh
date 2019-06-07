#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# The failure pointed out in http://gmt.soest.hawaii.edu/boards/1/topics/7776
# The axes labels and annotations are 180 out of phase.
#
ps=rotalign.ps

gmt set MAP_ANNOT_ORTHO ""
gmt psbasemap -R0/5/0/20/0/10 -JX3.5i/5i -JZ3i -Bxa1+l"x-axis" -Bya5+l"y-axis" -Bza2+l"z-axis" -P -K -BEWSNZ -pz60/30 > $ps
gmt psbasemap -R -JX -JZ -Bxa1+l"x-axis" -Bya5+l"y-axis" -Bza2+l"z-axis" -O -BEWSNZ -pz-60/30 -Y4.5i >> $ps
