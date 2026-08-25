#!/usr/bin/env bash
#
# Check two custom symbols symbols with new variables and text capabilities

ps=custom_azim.ps

cat << EOF > t.txt
5	25	0	20
10	25	10	20
15	25	20	20
5	20	30	20
10	20	40	20
15	20	50	20
5	15	60	20
10	15	70	20
15	15	80	20
EOF

cat << EOF > dip2.def
# Macro for geologic strike/dip takes strike (azimuth) and dip (angle) from input file
N:	1	a
# First rotate so strike is horizontal
\$1	R
# Draw the strike line
-0.5	0	M
0.5	0	D
S
# Draw the dip line
0	0	M
0	0.25	D
S
# Fill the dip vector head
0	0.25	M
-0.025	0.20	D
0	0.22	D
0.025	0.20	D
EOF
cat << EOF > dip3.def
# Macro for geologic strike/dip takes strike (azimuth) and dip (angle) from input file
N:	1	o
# First rotate so strike is horizontal
\$1	R
# Draw the strike line
-0.5	0	M
0.5	0	D
S
# Draw the dip line
0	0	M
0	0.25	D
S
# Fill the dip vector head
0	0.25	M
-0.025	0.20	D
0	0.22	D
0.025	0.20	D
EOF
gmt psxy -R0/20/10/30 -JM4i -P -K -Bag -BWSne -W1.5p,red -Skdip2/1i -Xc t.txt > $ps
gmt pstext -R -J -O -K -F+f16p+jTL+cTL+t"AZIMUTH 0-80" -Dj0.1i -Gwhite -W0.25p >> $ps
gmt psxy -R -JX4i -Bag -BWSne -W1.5p,red -Skdip3/1i t.txt -O -K -Y4.75i >> $ps
gmt pstext -R -J -O -F+f16p+jTL+cTL+t"ANGLE 0-80" -Dj0.1i -Gwhite -W0.25p >> $ps
