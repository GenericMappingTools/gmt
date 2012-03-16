#!/bin/bash
#       $Id$
#
# Check front symbols

header "Test psxyz with front symbols"

psbasemap -R0/7/0/8 -Jx1i -P -B0 -K -Xc > $ps
cat << EOF > t.txt
0	0	0
10	20	0
EOF
# Centered symbols using fixed interval, then same with just 1 centered symbol
psxyz -R-2/12/-2/22 -JM1i -O -K -W1p -Gred -Sf0.4i/0.1i+b t.txt -X0.25i -Y0.4i -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+c -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+f -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf0.4i/0.1i+t -X1i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf-1/0.1i+b -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf-1/0.1i+c -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf-1/0.1i+f -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gred -Sf-1/0.1i+t -X1i t.txt -p150/35 >> $ps
# Right symbols using fixed interval, then same with just 1 centered symbol
psxyz -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+b+r -X-4.5i -Y2.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+c+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+f+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+s+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf0.4i/0.1i+t+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf-1/0.1i+b+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf-1/0.1i+c+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf-1/0.1i+f+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf-1/0.1i+s+r -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gblue -Sf-1/0.1i+t+r -X0.5i t.txt -p150/35 >> $ps
# Left symbols using fixed interval, then same with just 1 centered symbol
psxyz -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+b+l -X-4.5i -Y2.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+c+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+f+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+s+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf0.4i/0.1i+t+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+b+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+c+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+f+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+s+l -X0.5i t.txt -p150/35 >> $ps
psxyz -R -J -O -K -W1p -Gyellow -Sf-1/0.1i+t+l -X0.5i t.txt -p150/35 >> $ps
psxy -R -J -O -T >> $ps

pscmp
