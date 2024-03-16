#!/usr/bin/env bash
# Because of issue #1225
ps=bezvec.ps
echo "168 53
174 51.5
189 54" > points

# As is
gmt psxy points -JQ180/57.5/6i -R160/200/50/65 -W0.1c,blue+s+ve0.4c -A -Bx10 -By5 -BswNE -P -K > $ps
gmt psxy points -J -R -W2p,black+s+ve0.4c -P -K -O >> $ps
gmt psxy points -J -R -W0.5p,red+ve0.4c -A -P -K -O >> $ps
gmt psxy points -J -R -W0.5p,pink+s -P -K -O >> $ps
gmt psxy points -J -R -Sx0.5c -Ggreen -P -O -K >> $ps
# Resampled
gmt sample1d -I200k -AR points > p.txt
gmt psxy p.txt -J -R -W0.1c,blue+s+ve0.4c -A -Bx10 -By5 -BswNE -O -K -Y5i >> $ps
gmt psxy p.txt -J -R -W2p,black+s+ve0.4c -P -K -O >> $ps
gmt psxy p.txt -J -R -W0.5p,red+ve0.4c -A -P -K -O >> $ps
gmt psxy p.txt -J -R -W0.5p,pink+s -P -K -O >> $ps
gmt psxy p.txt -J -R -Sx0.5c -Ggreen -P -O >> $ps
