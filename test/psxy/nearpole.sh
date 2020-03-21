#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# This is a follow up on topic http://gmt.soest.hawaii.edu/boards/1/topics/7166?r=7203#message-7203
# submitted by Sabin.  The problem polygon (Amundsen Terrane) is not polar but is very very close to the
# S pole and elongated almost 180 degrees way from the map center.  Hence it ends up very
# close to the periodib boundary in the -JW projection.  The fill clearly fails with
# a line across E-W while the outline is barely visible in the bottom and third panel.
ps=nearpole.ps
gmt psxy -R-180/180/-90/-60 -JW115/7i -Wfaint,red amundsen.txt -B+gyellow -P -K -Xc -Y0.5i > $ps
gmt psxy -R -J -Gred amundsen.txt -B+gyellow -O -K -Y1i >> $ps
gmt psxy -R -J -Wfaint,red amundsen.txt -Bxafg -Byafg10 -O -K -Y1i --MAP_FRAME_PEN=faint >> $ps
gmt psxy -R -J -Gred amundsen.txt -Bafg -O -K -Y1.25i  --MAP_FRAME_PEN=faint >> $ps
gmt psxy -R-180/180/-90/-80 -JA115/-90/5i -Bafg -O -K -Gred -Wfaint amundsen.txt -Y1.25i -X1i >> $ps
gmt psxy -R -J -O -Wfaint,blue -A << EOF >> $ps
-65	-90
-65	-80
EOF
