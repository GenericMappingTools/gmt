#!/bin/bash
# Test all gmt psrose region and types

ps=gallery.ps
common0="az_length.txt -: -S2.5c -F -L -Ggray -R0/150/-90/90 -Bxg25 -Byg90 -BWESN -O -K"
common1="az_length.txt -: -S2.5c -F -L -Ggray -R0/150/0/180 -Bxg25 -Byg90 -BWESN -O -K"
common0n="az_length.txt -: -S2.5c -F -L -Ggray -R0/5/-90/90 -Bxg1 -Byg90 -BWESN -O -K"
common1n="az_length.txt -: -S2.5c -F -L -Ggray -R0/5/0/180 -Bxg1 -Byg90 -BWESN -O -K"
common="az_length.txt -: -S2.5c -F -L -Ggray -R0/150/0/360 -Bxg25 -Byg90 -BWESN -O -K"
commonn="az_length.txt -: -S2.5c -F -L -Ggray -R0/1200/0/360 -Bxg200 -Byg90 -BWESN -O -K"
commonu="az_length.txt -: -S2.5c -F -L -R0/5/0/360 -Bxg1 -Byg190 -BWESN -O -K"
commonun="az_length.txt -: -S2.5c -F -L -R0/1/0/360 -Bxg0.2 -Byg190 -BWESN -O -K"
# Set up blank plot
gmt psxy -R0/5/0/5 -Jx1c -P -K -T > $ps
echo "2.5 -0.5 Sector Diagrams" | gmt pstext -R -J -O -K -N -F+jCT+f12p >> $ps
# 1st column: Default diagram
#  Row 1: Default plot
echo "-0.5 2.5 -R0/360/..." | gmt pstext -R0/5/0/5 -J -O -K -N -F+a90+jCB+f12p >> $ps
gmt psrose $common -A5 >> $ps
#  Row 2: Apply -T
echo "-0.5 2.5 -R0/360/... -T" | gmt pstext -R0/5/0/5 -J -O -K -N -F+a90+jCB+f12p -Y5.5c >> $ps
gmt psrose $common -A5 -T >> $ps
#  Row 3: Apply -Zu -T
echo "-0.5 2.5 -R0/360/... -Zu -T" | gmt pstext -R0/5/0/5 -J -O -K -N -F+a90+jCB+f12p -Y5.5c >> $ps
gmt psrose $commonn -A5 -Zu -T >> $ps
#  Row 4: Apply -R-90/90...
echo "-0.5 0 -R-90/90/..." | gmt pstext -R0/5/0/5 -J -O -K -N -F+a90+jLB+f12p -Y6c >> $ps
gmt psrose $common0 -A5 >> $ps
#  Row 5: Apply -R0/180...
echo "-0.5 0 -R0/180/..." | gmt pstext -R0/5/0/5 -J -O -K -N -F+a90+jLB+f12p -Y3.5c >> $ps
gmt psrose $common1 -A5 >> $ps
# 2nd column: Rose diagram
#  Row 1: Default plot
echo "2.5 -0.5 Rose Diagrams" | gmt pstext -R0/5/0/5 -J -O -K -N -F+jCT+f12p -Y-20.5c -X6c >> $ps
gmt psrose $common -A5r >> $ps
#  Row 2: Apply -T
gmt psrose $common -A5r -T -Y5.5c >> $ps
#  Row 3: Apply -Zu -T
gmt psrose $commonn -A5r -Zu -T -Y5.5c >> $ps
#  Row 4: Apply -R-90/90...
gmt psrose $common0 -A5r -Y6c >> $ps
#  Row 5: Apply -R0/180...
gmt psrose $common1 -A5r -Y3.5c >> $ps
# 3rd column: Windrose diagram
echo "2.5 -0.5 Windrose Diagrams" | gmt pstext -R0/5/0/5 -J -O -K -N -F+jCT+f12p -Y-20.5c -X6c >> $ps
#  Row 1: Default plot
gmt psrose $commonu >> $ps
#  Row 2: Apply -T
gmt psrose $commonu -T -Y5.5c >> $ps
#  Row 3: Apply -Zu -T
gmt psrose $commonun -Zu -T -Y5.5c >> $ps
#  Row 4: Apply -R-90/90...
gmt psrose $common0n -Y6c >> $ps
#  Row 5: Apply -R0/180...
gmt psrose $common1n -Y3.5c >> $ps
# Finalize
gmt psxy -R -J -O -T >> $ps
