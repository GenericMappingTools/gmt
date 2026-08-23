#!/usr/bin/env bash
# Calibration plot for the stereonet module.
# Left net: eight fault planes given as strike/dip (right-hand rule); we draw the
# cyclographic trace of each plane plus its pole, so each pole must sit 90 degrees
# from every point of its own trace.
# Right net: the same planes given as dip direction/dip on a Wulff net, plotted on
# the upper hemisphere, so the picture must be the left one point-reflected through
# the center (and stretched, since equal-angle differs from equal-area).
# Bottom net: lines given as trend/plunge.  A plunge of 90 must land in the center
# and a plunge of 0 on the perimeter at the azimuth of its trend.
ps=stereonet.ps

cat << EOF > planes.txt
90	30
180	45
270	60
0	15
30	45
120	48
225	27
350	80
EOF
cat << EOF > lines.txt
0	0
90	30
180	60
270	90
45	45
EOF

# Strike/dip on a Schmidt (equal-area) net
gmt psstereonet planes.txt -JA8c -W1p,red -Sx0.3c -L1p,blue -P -K -X2c -Y17c \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray > $ps
# Dip direction/dip on a Wulff (equal-angle) net, upper hemisphere, coarser ring
gmt psstereonet planes.txt -JS8c -Td+u -A45/15 -W1p,red -Sx0.3c -L1p,blue -O -K -X10c \
	--MAP_GRID_PEN_PRIMARY=0.25p,gray >> $ps
# Trend/plunge of lines, no azimuth ring, colored by plunge
gmt makecpt -Cturbo -T0/90 > t.cpt
awk '{print $1, $2, $2}' lines.txt | gmt psstereonet -JA8c -Tl -A0 -Ct.cpt -Sc0.4c \
	-Bpg15 -Bsg45 -O -X-5c -Y-11c >> $ps
