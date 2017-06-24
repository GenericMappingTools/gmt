#!/bin/bash
# Test mapproject's distance and time calculations
ps=waypoints.ps
gmt mapproject -G+un+i+a -Af -Z20+i+a+t2017-01-01T --TIME_UNIT=h ${src:-.}/waypoints.txt -: -o2-4,7 --FORMAT_FLOAT_OUT=%.1f > tmp
gmt psxy -R181/185/-7:30/-2 -JM6.5i -P -Baf -BWSne -W0.25p ${src:-.}/waypoints.txt -: -K > $ps
gmt psxy -R -J -O -K -Sc0.2c -Gblue ${src:-.}/waypoints.txt -: >> $ps
awk '{if (NR == 2) print $2, $1}' ${src:-.}/waypoints.txt | gmt psxy -R -J -O -K -Sa0.4c -Gred >> $ps
awk '{if (NR > 2) print $2, $1, $3}' ${src:-.}/waypoints.txt | gmt pstext -R -J -O -K -F+f9p+r100 -D0/0.4c >> $ps
cat << EOF > legend
H 20 Helvetica Map Legend
G 4p
N 0.75 0.9 1.1 0.9
V 0 0.25p
L 16 0 C WPT#
L 16 0 C Heading
L 16 0 C Distance
L 16 0 C Time
D 0 0.5p
EOF
awk '{if (NR > 1) printf "L 16 0 C P-%d\nL 16 0 R %s\\312\nL 16 0 R %s nm\nL 16 0 R %s hr\n", int($1), $2, $3, $4}' tmp >> legend
echo "V 0 0.25p" >> legend
gmt pslegend -R -J -O -DjTL+w3.75i+o0.1i -F+p1p+gwhite+s legend >> $ps
