#!/bin/bash
#       $Id$
#
# Check front symbols

. ../functions.sh
header "Test psxy with front symbols"
ps=front.ps
psbasemap -R0/6/0/8 -Jx1i -P -B0 -K -Xc > $ps
cat << EOF > t.txt
0	0
10	20
EOF
# Centered symbols using fixed interval, then same with just 1 centered symbol
psxy -R-2/12/-2/22 -JM1i -O -K -W1p -Gred -Sf0.4i/0.1i+b t.txt -X0.25i -Y0.4i >> $ps
psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+c -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+f -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+t -X1i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+b -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+c -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+f -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gred -Sf-1/0.1i+t -X1i t.txt >> $ps
# Right symbols using fixed interval, then same with just 1 centered symbol
psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+b+r -X-4.5i -Y2.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+c+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+f+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+s+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+t+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+b+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+c+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+f+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+s+r -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gblue -Sf-1/0.1i+t+r -X0.5i t.txt >> $ps
# Left symbols using fixed interval, then same with just 1 centered symbol
psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+b+l -X-4.5i -Y2.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+c+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+f+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+s+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+t+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+b+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+c+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+f+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+s+l -X0.5i t.txt >> $ps
psxy -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+t+l -X0.5i t.txt >> $ps
psxy -R -J -O -T >> $ps

pscmp
