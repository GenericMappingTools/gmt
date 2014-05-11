#!/bin/bash
#               GMT EXAMPLE 41
#               $Id$
#
# Purpose:      Illustrate typesetting of legend with table
# GMT progs:    pscoast, pslegend, psxy
# Unix progs:   cat, rm
#
ps=example_41.ps

gmt gmtset FONT_ANNOT_PRIMARY 12p  FONT_LABEL 12p

cat <<EOF > table.txt
# Legend test for gmt pslegend
# G is vertical gap, V is vertical line, N sets # of columns, D draws horizontal line,
#
#G 0.04i
H 24 Times-Roman Ten Largest Cities in North America
D 1p
N 4 6 22 15 19 18 8 8
V 0.25p
L - - C #
S
L - - C City Name
L - - C Country
L - - C Population
L - - C Climate
L - - C WC?
L - - C OL?
D 1p +
F lightgreen 
L - - C 1
S C c 0.1i red 0.25p
L - - R Mexico City
L - - R Mexico
L - - R 8,851,080
L - - R Tropical
L - - C Y
L - - C Y
F lightyellow 
L - - C 2
S C c 0.1i orange 0.25p
L - - R New York City
L - - R USA
L - - R 8,405,837
L - - R Tempered
L - - C Y
L - - C N
L - - C 3
S C c 0.1i yellow 0.25p
L - - R Los Angeles
L - - R USA
L - - R 3,904,657
L - - R Subtropical
L - - C Y
L - - C Y
F lightblue 
L - - C 4
S C c 0.1i green 0.25p
L - - R Toronto
L - - R Canada
L - - R 2,795,060
L - - R Tempered
L - - C N
L - - C N
F lightyellow 
L - - C 5
S C c 0.1i blue 0.25p
L - - R Chicago
L - - R USA
L - - R 2,714,856
L - - R Tempered
L - - C Y
L - - C N
L - - C 6
S C c 0.1i cyan 0.25p
L - - R Houston
L - - R USA
L - - R 2,714,856
L - - R Subtropical
L - - C N
L - - C N
F lightred 
L - - C 7
S C c 0.1i magenta 0.25p
L - - R Havana
L - - R Cuba
L - - R 2,106,146
L - - R Tropical
L - - C N
L - - C N
F lightblue 
L - - C 8
S C c 0.1i white 0.25p
L - - R Montreal
L - - R Canada
L - - R 1,649,519
L - - R Tempered
L - - C N
L - - C Y
F lightgreen 
L - - C 9
S C c 0.1i red 0.25p
L - - R Ecatepec
L - - R Mexico
L - - R 1,658,806
L - - R Tropical
L - - C N
L - - C N
L - - R
S
L - - R de Morelos
L - - R
L - - R
L - - R
L - - C
L - - C
F lightyellow 
L - - C 10
S C c 0.1i cyan 0.25p
L - - R Philadelphia
L - - R USA
L - - R 1,553,165
L - - R Tempered
L - - C N
L - - C N
D 1p -
V 1p
F - 
N 1
L 9 6 R Information from Wikipedia
G 0.05i
T Many of these North American cities have hosted the Football World Cup
T (WC) and some have hosted the Olympics (OL).  The rest is just some
T basic information about each city, such as climate and population.
T Of course, this is all an excuse to demonstrate variable-width tables
T and coloring, as well as a custom number-symbol on the map.
G 0.1i
EOF
cat << EOF > t.cpt
1	red
2	orange
3	yellow
4	green
5	bisque
6	cyan
7	magenta
8	white
9	gray
EOF
cat << EOF > numbcircle.def
N: 1 s
0 0 1 c
0 0 0.7 \$1 l
EOF
gmt pscoast -R130W/50W/8N/57N -JM5.6i -B0 -P -K -Glightgray -Sazure1 -A1000 -Xc -Y1.2i --MAP_FRAME_TYPE=plain > $ps
gmt pscoast -R -J -O -K -FUS+glightyellow+pfaint -FCU+glightred+pfaint -FMX+glightgreen+pfaint -FCA+glightblue+pfaint -V >> $ps
gmt pscoast -R -J -O -K -N1/1p,darkred -A1000/2/2 -Wfaint -Cazure1 -V >> $ps
gmt psxy -R -J -O -K -Sknumbcircle/0.1i -Ct.cpt -W0.25p -: << EOF >> $ps
40.7127N, 74.0059W	2	2
34.0500N, 118.2500W	3	3
43.7000N, 79.4000W	4	4
41.8819N, 87.6278W	5	5
29.7628N, 95.3831W	6	6
23.1333N, 82.3833W	7	7
45.5000N, 73.5667W	8	8
19.6097N, 99.0600W	1	9
39.9500N, 75.1667W	9	10
19.4333N, 99.1333W	1	1
EOF
gmt pslegend -R0/6/0/9.1 -Jx1i -Dx3i/4.65i/5.6i/BC -C0.05i -L1.2 -F+p+gsnow1 -B0 -X-0.2i -Y-0.2i -O -Vl table.txt >> $ps
rm -f table.txt t.cpt numbcircle.def
