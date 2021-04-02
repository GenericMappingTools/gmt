#!/usr/bin/env bash
# Test gmt psrose with -Em and basemap amd -Ccpt

ps=sector_vpdf.ps

# Output a von Mises distribution centered on 60 with kappa = 4
gmt math -T0/360/1 T 60 4 VPDF = t.txt
# First plot the data set with best-fitting von Mises
gmt psrose a.txt -R0/1/0/360 -Bx0g0.2 -Byg60 -B+glightgreen -A5 -Glightblue -W1p -: -S -N0+p1p -JX4i -P -K -Xc -Em > $ps
# Then plot the synthetic set with best-fitting von Mises
gmt psrose t.txt -R -Bx0g0.2 -Byg60 -B+glightgreen -A5 -Glightblue -W1p -: -S -N0+p1p -J -O -Y4.75i -Em >> $ps
