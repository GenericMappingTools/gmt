#!/bin/bash
#		GMT EXAMPLE 30
#		$Id: job30.sh,v 1.3 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Show graph mode and math angles
# GMT progs:	gmtmath, psbasemap, pstext and psxy
# Unix progs:	echo, rm
#
# Draw generic x-y axes with arrows
. ../functions.sh
ps=../example_30.ps

psbasemap -R0/360/-1.25/1.75 -JX8i/6i -B90f30:,-\\312:/1g10:."Two Trigonometric Functions":WS -K \
	-U"Example 30 in Cookbook" --BASEMAP_TYPE=graph --VECTOR_SHAPE=0.5 > $ps

# Draw sine an cosine curves

gmtmath -T0/360/0.1 T COSD = | psxy -R -J -O -K -W2p >> $ps
gmtmath -T0/360/0.1 T SIND = | psxy -R -J -O -K -W2p,. --PS_LINE_CAP=round >> $ps

# Indicate the x-angle = 120 degrees
psxy -R -J -O -K -W0.5p,- << EOF >> $ps
120	-1.25
120	1.25
EOF

pstext -R -J -O -K -Dj0.05i -N << EOF >> $ps
360 1 18 0 4 RB x = cos(@%12%a@%%)
360 0 18 0 4 RB y = sin(@%12%a@%%)
120 -1.25 14 0 4 LB 120\\312
370 -1.35 24 0 12 LT a
-5 1.85 24 0 4 RT x,y
EOF

# Draw a circle and indicate the 0-70 degree angle

echo 0 0 | psxy -R-1/1/-1/1 -Jx1.5i -O -K -X3.625i -Y2.75i -Sc2i -W1p -N >> $ps
psxy -R -J -O -K -m -W1p << EOF >> $ps
> x-gridline  -W0.25p
-1	0
1	0
> y-gridline  -W0.25p
0	-1
0	1
> angle = 0
0	0
1	0
> angle = 120
0	0
-0.5	0.866025
> x-projection -W2p
-0.3333	0
0	0
> y-projection -W2p
-0.3333	0.57735
-0.3333	0
EOF

pstext -R -J -O -K -Dj0.05i << EOF >> $ps
-0.16666 0 12 0 4 CT x
-0.3333 0.2888675 12 0 4 RM y
0.22 0.27 12 -30 12 CB a
-0.33333 0.6 12 30 4 LB 120\\312
EOF

echo 0 0 0 120 | psxy -R -J -O -Sml1i -W1p >> $ps

rm -f .gmt*
