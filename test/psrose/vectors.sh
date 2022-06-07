#!/usr/bin/env bash
# Test gmt psrose windrose with or without vector heads

ps=vectors.ps
data=`gmt which -Gc @azimuth_lengths.txt`
awk '{if ((NR%50) == 0) print $0}' $data > subset.txt
common0n="subset.txt -: -JX7c -F -L -Ggray -R0/3/-90/90 -Bxg1 -Byg90 -BWESN -O -K"
common1n="subset.txt -: -JX7c -F -L -Ggray -R0/3/0/180 -Bxg1 -Byg90 -BWESN -O -K"
commonn="subset.txt  -: -JX7c -F -L -Ggray -R0/750/0/360 -Bxg200 -Byg90 -BWESN -O -K"
commonu="subset.txt  -: -JX7c -F -L -R0/3/0/360 -Bxg1 -Byg90 -BWESN -O -K"
# Set up blank plot
gmt psxy -R0/5/0/5 -Jx1c -P -K -T > $ps
gmt set MAP_VECTOR_SHAPE 0.5
# 3Windrose diagram
#  col 1: Default plot
gmt psrose $commonu >> $ps
#  Row 2: Apply -T
gmt psrose $commonu -T -Y7.5c >> $ps
#  Row 3: Apply -R-90/90...
gmt psrose $common0n -Y7.5c >> $ps
#  Row 4: Apply -R0/180...
gmt psrose $common1n -Y5c >> $ps
#  col 2: vector plot
#  Row 1: Default plot
gmt psrose $commonu -M0.5c+e+gorange+n1i/0+h0.5 -Y-20c -X9c >> $ps
#  Row 2: Apply -T
gmt psrose $commonu -T -Y7.5c -M0.5c+b+e+gorange+n1i/0+h0.5 >> $ps
#  Row 3: Apply -R-90/90...
gmt psrose $common0n -Y7.5c -M0.5c+e+gorange+n1i/0+h0.5 >> $ps
#  Row 4: Apply -R0/180...
#gmt psrose $common1n -Y5c -M0.5c+e+gorange+n1i/0 >> $ps
gmt psrose $common1n -Y5c -M+ >> $ps
# Finalize
gmt psxy -R -J -O -T >> $ps
