#!/usr/bin/env bash
# Place a line with text along the straight lines and use font fill and font outline
ps=quotedoutline.ps
printf "0 0\n1 1\n" > t.txt
# Regular filled text
gmt psxy t.txt -JX17c/8c -R0/1/0/1 -Sqn1:+l"Test String"+f60p,Times-Bold,green -P -B0 -K -Y1.5c > $ps
# Fill, then draw outline
gmt psxy t.txt -J -R -Sqn1:+l"Test String"+f60p,Times-Bold,green=2p,red -O -K -B0 -Y8.5c >> $ps
# Draw outline, then fill text
gmt psxy t.txt -J -R -Sqn1:+l"Test String"+f60p,Times-Bold,green=~2p,red -O -B0 -Y8.5c >> $ps
