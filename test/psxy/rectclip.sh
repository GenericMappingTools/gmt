#!/bin/bash

# Test case for line far outside a small region.  First point
# is > 180 points away so is seen as -179 away while second is
# +179 away. The projected x coordinates are thus far outside
# on either side, tricking the rect_clip function to compute
# two crossings and drawing the line.  Added new check to try
# to avoid this case - monitoring to see if it is OK; see
# gmt_map.c's gmt_rect_overlap function. P. Wessel 7/31/2014
# There should be no lines crossing these two maps.
ps=rectclip.ps

cat << EOF > t.txt
107	62
105	62
EOF
#
gmt select -R-88.9/-14.1/-58.5993313003/6.9895410772r -fg t.txt > a.txt
gmt psxy -JL-73.5/-10.6/-14.16/-14.24/6i -R-88.9/-14.1/-58.5993313003/6.9895410772r -P -Ba5f1g5 -W2p,red t.txt -Xc -K > $ps
gmt psxy -J -R -O -Ba5f1g5  -W2p,red a.txt -Y5i >> $ps
