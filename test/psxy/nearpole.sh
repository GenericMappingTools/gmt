#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# This is a follow up on topic http://gmt.soest.hawaii.edu/boards/1/topics/7166?r=7203#message-7203
# submitted by Sabin.  The problem polygon (Amundsen Terrane) is not polar but is very very close to the
# S pole and elongated almost 180 degrees way from the map center.  Hence it ends up very
# close to the periodib boundary in the -JW projection.  The fill clearly fails with
# a line across E-W while the outline is barely visible in the bottom and third panel.
#
# Update 8/11/2020 PW: The wrapping across E-W occurs for several ranges of longitude, but not
# the 115 given in this test. Changing to 114 to highlight the problem.  I made a movie as well
# from this main.sh:
# gmt begin
# 	gmt plot -R-180/180/-90/-60 -JW${MOVIE_COL0}/6i -Wfaint,red amundsen.txt -B+gblue -X0.1i -Y0.1i
#	gmt plot -Gred amundsen.txt -B+gblue -Y1i
# gmt end
#
# and ran
# gmt movie main.sh -T0/360/0.1 -C6.2ix2ix300 -Fmp4 -M50 -Namund -Lc0 -V
# Which showed problems for central longs 112.9-114.7. 123.4-137? where things are wrapping.
ps=nearpole.ps
gmt psxy -R-180/180/-90/-60 -JW114/7i -Wfaint,red amundsen.txt -B+gyellow -P -K -Xc -Y0.5i > $ps
gmt psxy -R -J -Gred amundsen.txt -B+gyellow -O -K -Y1i >> $ps
gmt psxy -R -J -Wfaint,red amundsen.txt -Bxafg -Byafg10 -O -K -Y1i --MAP_FRAME_PEN=faint >> $ps
gmt psxy -R -J -Gred amundsen.txt -Bafg -O -K -Y1.25i  --MAP_FRAME_PEN=faint >> $ps
gmt psxy -R-180/180/-90/-80 -JA114/-90/5i -Bafg -O -K -Gred -Wfaint amundsen.txt -Y1.25i -X1i >> $ps
gmt psxy -R -J -O -Wfaint,blue -A << EOF >> $ps
-65	-90
-65	-80
EOF
