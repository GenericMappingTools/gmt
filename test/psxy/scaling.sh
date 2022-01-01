#!/usr/bin/env bash
# Test case for bug in -i with psxy where there is a conversion from datavalue
# assumed to be cm before the scaling takes place.  The issue is this line
# echo 0 0 150 | gmt psxy -Scc -i0,1,2+s0.01+o-0.5
# where we are saying that 150 should be modified via -i settings to yield a
# value that we wish to consider as diameter of a circle in cm.  Instead, the
# 150 is considered to be in cm, converted to 59.0555 (150/2.54), then scaled
# as z = 59.0555 * 0.01 - 0.5 = 0.09 and then used as cm (i.e, ~ 1 mm)
ps=scaling.ps
#
# LL: Plot 1 inch and 1 cm circles
gmt psxy -R-2/2/-2/4 -Jx1c -P -Baf -Sc -Gblack -K -X5c -Y8c << EOF > $ps
0 0 1c
0 2 1i
EOF
# LR
# Convert the value of 100 to 1 cm
echo 0 0 100 | gmt psxy -R -J  -Baf -O -K -Scc -i0,1,2+s0.01 -Gblack -X6c >> $ps
# Convert the value of 100 to 1 inch
echo 0 2 100 | gmt psxy -R -J -O -K -Sci -i0,1,2+s0.01 -Gblack >> $ps
# UL
# BUG: Convert the value of 150 to 1 cm via z = (150 - 50) * 0.01 = 150 * 0.01 - 0.5
# The 150 is converted to inch (prematurely) then scaled and offset and may even go negative
# Next line faked the answer to build a test ps:
# echo 0 0 1 | gmt psxy -R -J  -Baf -O -K -Scc -Gblack -X-6c -Y8c >> $ps
# Next line is the offending line:
echo 0 0 150 | gmt psxy -R -J  -Baf -O -K -Scc -i0,1,2+s0.01+o-0.5 -Gblack -X-6c -Y8c >> $ps
# Convert the value of 150 to 1 inch via z = (150 - 50) * 0.01 = 150 * 0.01 - 0.5
echo 0 2 150 | gmt psxy -R -J -O -K -Sci -i0,1,2+s0.01+o-0.5 -Gblack >> $ps
# UR
# Simulate what we want with awk, first for cm
echo 0 0 150 | awk '{print $1, $2, ($3-50)*0.01}' | gmt psxy -R -J -Baf -O -K -Scc -Gblack -X6c >> $ps
# Simulate what we want with awk, for inch
echo 0 2 150 | awk '{print $1, $2, ($3-50)*0.01}' | gmt psxy -R -J -O -Sci -Gblack >> $ps
