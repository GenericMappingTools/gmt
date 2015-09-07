#!/bin/bash
# Test pstext options for accepting or modifiying text using a format
ps=labeler.ps
echo 3 2.5 > t.txt
gmt convert points.txt -o0,1 > p.txt
gmt psxy -R0/7/-0.5/6.5 -JX6i -P -Baf -Sc0.25i -Gred -Wfaint points.txt -K > $ps
gmt pstext -R -J -O -K -F+T"%2.2d"+f10p,Helvetica-Bold,white p.txt >> $ps
gmt pstext -R -J -O -K -F+T"z = %.1f\312"+f9p,Times-Italic+jTC -Dj0/0.2i points.txt >> $ps
gmt pstext -R -J -O -K -F+t"Some text label"+f24p,Courier-Bold t.txt >> $ps
gmt psxy -R -J -O -T >> $ps
