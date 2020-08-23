#!/usr/bin/env bash
#
# Plot psxy front symbols for use on man page

ps=GMT_base_symbols7.ps

cat << EOF > t.txt
0	0
10	20
EOF
# Centered symbols using fixed interval, then same with just 1 centered symbol
gmt psxy -R-2/12/-10/30 -JX1i -P -K -W1p -Glightblue -Sf0.4i/0.07i+b t.txt > $ps
gmt psxy -R -J -O -K -W1p -Glightred -Sf0.4i/0.1i+c+r -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+f -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Sf0.4i/0.2i+S+r -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblack -Sf0.2i/0.05i+t+l -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+b -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+c -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+f -X0.6i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Sf-1/0.4i+S+l -X0.6i t.txt >> $ps
gmt psxy -R -J -O -W1p -Glightorange -Sf-1/0.1i+t -X0.6i t.txt >> $ps
