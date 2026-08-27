#!/usr/bin/env bash
# Plot trend/plunge lines (-Tl) colored by plunge via a CPT, with no
# azimuth ring (-A0).  A plunge of 90 must land in the center and a
# plunge of 0 on the perimeter at the azimuth of its trend.
ps=lines_cpt.ps

cat << EOF > lines.txt
0	0
90	30
180	60
270	90
45	45
EOF

gmt makecpt -Cturbo -T0/90 > t.cpt
awk '{print $1, $2, $2}' lines.txt | gmt psstereonet -JA8c -Tl -A0 -Ct.cpt -Sc0.4c \
	-Bpg15 -Bsg45 -P > $ps
