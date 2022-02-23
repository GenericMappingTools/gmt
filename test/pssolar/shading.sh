#!/usr/bin/env bash
# Test pssolar with shading
ps=shading.ps
gmt pscoast -Rg -JG30/30/6i -P -Glightbrown -Sazure1 -Dc -A50000 -K -Bg -Xc -Y0.5i > $ps
gmt pssolar -R -J -O -K -Gblack@70 -T+d2012-6-12T18:00:00 >> $ps
gmt pscoast -Rg -JH0/6i -O -Glightbrown -Sazure1 -Dc -A50000 -K -Bg -Y6.5i >> $ps
gmt pssolar -R -J -O -Gblack@70 -T+d2012-6-12T18:00:00 >> $ps
