#!/usr/bin/env bash
#
# Check front symbols

ps=front.ps

gmt psbasemap -R0/7/0/8 -Jx1i -P -B0 -K -X0.5i > $ps
cat << EOF > t.txt
0	0
10	20
EOF
# Centered symbols using fixed interval, then same with just 1 centered symbol
gmt psxy -R-2/12/-3/23 -JM1i -O -K -W1p -Gred -Sf0.4i/0.1i+b t.txt -X0.25i -Y0.4i >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+c -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+f -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Sf0.6i/0.3i+S+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+t -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+v -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+b -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+c -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+f -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Sf-1/0.4i+S+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+t -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+v -X0.5i t.txt >> $ps
# Right symbols using fixed interval, then same with just 1 right-side symbol
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+b+r -X-5.5i -Y2.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+c+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+f+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.6i/0.3i+s+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+t+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+v+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+b+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+c+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+f+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.4i+s+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+t+r -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+v+r -X0.5i t.txt >> $ps
# Left symbols using fixed interval, then same with just 1 left-side symbol
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+b+l -X-5.5i -Y2.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+c+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+f+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.6i/0.3i+s+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+t+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+v+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+b+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+c+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+f+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.4i+s+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+t+l -X0.5i t.txt >> $ps
gmt psxy -R -J -O -W1p -Gyellow -Sf-1/0.1i+v+l -X0.5i t.txt >> $ps
