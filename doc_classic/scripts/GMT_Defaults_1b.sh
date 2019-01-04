#!/usr/bin/env bash
#
ps=GMT_Defaults_1b.ps
gmt set MAP_FRAME_TYPE plain FORMAT_GEO_MAP ddd:mm:ssF MAP_GRID_CROSS_SIZE_PRIMARY 0i \
	FONT_ANNOT_PRIMARY +8p MAP_ANNOT_OBLIQUE 1
gmt psbasemap -X1.5i -R-90/20/-55/25r -JOc-80/25.5/2/69/2.25i -Ba10f5g5 -P -K > $ps
gmt pstext -R0/2.25/0/2 -Jx1i -O -K -N -F+f7p,Helvetica-Bold,blue+j << EOF >> $ps
-0.15 0.15  RB MAP_ORIGIN_X
0.28  -0.2  LM MAP_ORIGIN_Y
-0.2   1.2  RB MAP_FRAME_PEN
2.4   -0.2  LT MAP_ANNOT_OBLIQUE
2.4    1.2  LB MAP_GRID_PEN_PRIMARY
2      1.4  LB MAP_ANNOT_MAX_ANGLE
2.35   0.5  LM MAP_ANNOT_MIN_SPACING
1      0.5  RT MAP_LINE_STEP
2.7    0.32 LM PS_PAGE_COLOR
2.7    0.18 LM PS_MEDIA
1.5     1.8 LB FONT_ANNOT_PRIMARY
EOF
echo -0.4 -0.4 | gmt psxy -R -J -O -K -Sc0.04i -Gblue -N >> $ps
gmt psxy -R -J -O -K -Sv0.06i+s+b+e -N -W0.5p,blue -Gblue << EOF >> $ps
0 0.1 -0.4 0.1
0.25 0 0.25 -0.4
2.3 0.1 2.3 0.9
EOF
gmt psxy -R -J -O -K -Sv0.06i+s+e -N -W0.5p,blue -Gblue << EOF >> $ps
-0.2 1.15 -0.03 1
2.35 -0.2 2.0 -0.05
2.35 1.2 1.95 1.1
1.95 1.4 1.4 1.4
1.05 0.5 1.65 0.75
3.25 0.25 3.6 0.25
1.45 1.8 1 1.6
EOF
gmt psxy -R -J -O -Wthinnest,- -X-0.5i -Y-0.5i << EOF >> $ps
>
0.1	0.1
0.1	0.6
>
0.1	0.1
0.75	0.1
EOF
