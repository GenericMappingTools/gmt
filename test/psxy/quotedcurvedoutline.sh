#!/usr/bin/env bash
# Place a line with text along the curved line (well, straight here but via +v) and use font fill and font outline
ps=quotedcurvedoutline.ps
gmt math -T0/1/0.01 T SQRT = t.txt
# Regular filled text
gmt psxy t.txt -JX17c/8c -R0/1/0/1 -Sqn1:+v+l"Test String"+f60p,Times-Bold,green -P -B0 -K -Y1.5c > $ps
# Fill, then draw outline
gmt psxy t.txt -J -R -Sqn1:+v+l"Test String"+f60p,Times-Bold,green=2p,red -O -K -B0 -Y8.5c >> $ps
# Draw outline, then fill text
gmt psxy t.txt -J -R -Sqn1:+v+l"Test String"+f60p,Times-Bold,green=~2p,red -O -B0 -Y8.5c >> $ps
