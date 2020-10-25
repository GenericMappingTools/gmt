#!/usr/bin/env bash
#
# Check vector symbols

ps=matharc.ps

gmt psbasemap -R0/6/0/3 -Jx1i -P -B1g1 -BWSne -K -Xc -p155/35 > $ps
gmt set MAP_VECTOR_SHAPE 1
# Math angle vectors
gmt psxyz -R -J -O -K -W1p -Gred -S -p155/35 << EOF >> $ps
0	0	0	1i	30	80	m0.2i
1	0	0	1i	30	80	m0.2i+b
2	0	0	1i	30	80	m0.2i+e+p-
3	0	0	1i	30	80	m0.2i+b+e+gorange
4	0	0	1i	30	80	m0.2i+b+l+p0.5p,blue
5	0	0	1i	30	80	m0.2i+e+r
EOF
# Right angles
gmt psxyz -R -J -O -K -W1p -Gred -S -p155/35 << EOF >> $ps
0.5	1.5	0	0.5i	0	90	M0.2i
1.5	1.5	0	0.5i	60	150	M0.2i+b
2.5	1.5	0	0.5i	120	210	M0.2i+e
3.5	1.5	0	0.5i	180	270	M0.2i+b+e
4.5	1.5	0	0.5i	240	330	M0.2i+b+l
5.5	1.5	0	0.5i	300	390	M0.2i+e+r
EOF
# Math angle vectors unfilled
gmt psxyz -R -J -O -K -W1p -S -p155/35 << EOF >> $ps
0	2	0	1i	30	80	m0.2i
1	2	0	1i	30	80	m0.2i+b
2	2	0	1i	30	80	m0.2i+e
3	2	0	1i	30	80	m0.2i+b+e
4	2	0	1i	30	80	m0.2i+b+l
5	2	0	1i	30	80	m0.2i+e+r
EOF
# Normalized by angle below
gmt psbasemap -R0/4/0/4 -J -O -B1g1 -BWSne -K -X1i -Y4i -p155/35 >> $ps
gmt psxyz -R -J -O -W1p -Gblack -Sm0.3i+b+e+n90 << EOF -p155/35 >> $ps
0	0	0	4.0i	0	90
0	0	0	3.6i	0	80
0	0	0	3.2i	0	70
0	0	0	2.8i	0	60
0	0	0	2.4i	0	50
0	0	0	2.0i	0	40
0	0	0	1.8i	0	35
0	0	0	1.6i	0	30
0	0	0	1.4i	0	25
0	0	0	1.2i	0	20
0	0	0	1.0i	0	15
0	0	0	0.8i	0	10
EOF
