#!/bin/bash
#	$Id$
#
# Tests gmt project in generating lines

ps=generate.ps

# First Cartesian lines

# E-W line
gmt project -C10/10 -A90 -G1 -L-9/11 -N > tt.xy
gmt psxy -R0/25/0/25 -JX4i -P -K -X2i tt.xy -W2p,red > $ps
echo 10 10 | gmt psxy -R -J -O -K -Sc0.1i -Gred >> $ps
echo 10 10 0.4i 0 90 | gmt psxy -R -J -O -K -Sm0.15i+b -W0.75p,red -Gred >> $ps
gmt pstext -R -J -F+f12p,Helvetica-Bold,red+jBL -O -K >> $ps <<< "21 11 E-W"
gmt pstext -R -J -F+f12p,Helvetica-Bold,red+jBL -O -K >> $ps <<< "12 12 90\312"

# 30 degrees azimuth
gmt project -C5/5 -A30 -G1 -L-3/12 -N > tt.xy
gmt psxy -R -J -O -K tt.xy -W2p,green >> $ps
echo 5 5 | gmt psxy -R -J -O -K -Sc0.1i -Ggreen >> $ps
echo 5 5 0.4i 60 90 | gmt psxy -R -J -O -K -Sm0.15i+b -W0.75p,green -Ggreen >> $ps
gmt pstext -R -J -F+f12p,Helvetica-Bold,green+jTR -O -K >> $ps <<< "3 2 -A30"
gmt pstext -R -J -F+f12p,Helvetica-Bold,green+jTR -O -K >> $ps <<< "6.6 9 30\312"

# Between two given points
gmt project -C15/5 -E2/20 -G1 -N > tt.xy
gmt psxy -R -J -O -K tt.xy -W2p,blue -B5g5 -BWSne >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gblue << EOF >> $ps
15	5
2	20
EOF

# Spherical test

# E-W line
gmt project -C10/10 -A90 -G10 -L-50/30 > tt.xy
gmt psxy -Rg -JA0/0/5i -O -K -X-0.5i -Y4.25i tt.xy -W2p,red >> $ps
echo 10 10 | gmt psxy -R -J -O -K -Sc0.1i -Gred >> $ps
gmt pstext -R -J -F+f12p,Helvetica-Bold,red+jBL -O -K >> $ps <<< "37 11 E-W"
# 30 degrees azimuth
gmt project -C5/5 -A30 -G10 -L-40/50 > tt.xy
gmt psxy -R -J -O -K tt.xy -W2p,green >> $ps
echo 5 5 | gmt psxy -R -J -O -K -Sc0.1i -Ggreen >> $ps
gmt pstext -R -J -F+f12p,Helvetica-Bold,green+jTR -O -K >> $ps <<< "-20 -30 -A30"

# Between two given points
gmt project -C15/5 -E-12/-40 -G10 > tt.xy
gmt psxy -R -J -O -K tt.xy -W2p,blue >> $ps
gmt psxy -R -J -O -K -Sc0.1i -Gblue << EOF >> $ps
15	5
-12	-40
EOF

# Point and rotation pole
gmt project -C15/15 -E85/40 -G10 -L-180/180 | gmt psxy -R -J -O -K -W0.25p,- >> $ps
gmt project -C15/15 -T85/40 -G10 -L-20/60 > tt.xy
gmt psxy -R -J -O -K tt.xy -W2p >> $ps
echo 15 15 | gmt psxy -R -J -O -K -Sc0.1i -Gblack >> $ps
echo 85 40 | gmt psxy -R -J -O -K -Sa0.1i -Gblack >> $ps
gmt pstext -R -J -F+f12p,Helvetica-Bold+jTR -O -K >> $ps <<< "85 35 P"
# The end
gmt psbasemap -R -J -O -B30g30 >> $ps

