#!/bin/sh
#	$Id: GMT_Defaults_1b.sh,v 1.2 2001-09-26 04:34:47 pwessel Exp $
#
gmtset BASEMAP_TYPE plain PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE 0i \
ANOT_FONT_SIZE +8p OBLIQUE_ANOTATION 1
psbasemap -R-90/20/-55/25r -JOc-80/25.5/2/69/2.25i -Ba10f5g5 -P -K > GMT_Defaults_1b.ps
pstext -R0/2.25/0/2 -Jx1i -O -K -N << EOF >> GMT_Defaults_1b.ps
-0.05 0.15 7 0 1 RB X_ORIGIN
0.28 -0.2 7 0 1 LM Y_ORIGIN
-0.2 1.2 7 0 1 RB FRAME_PEN
2.4 -0.2 7 0 1 LT OBLIQUE_ANOTATION
2.4 1.2 7 0 1 LB GRID_PEN
2 1.4 7 0 1 LB ANOT_MAX_ANGLE
2.35 0.5 7 0 1 LM ANOT_MIN_SPACING
1 0.5 7 0 1 RT LINE_STEP
2.7 0.32 7 0 1 LM PAGE_COLOR
2.7 0.18 7 0 1 LM PAGE_MEDIA
0.4 1.8 7 0 1 LB ANOT_FONT
1.3 1.8 7 0 1 LB ANOT_FONT_SIZE
EOF
echo -0.4 -0.4 | psxy -R -Jx -O -K -Sc0.04 -G0 -N >> GMT_Defaults_1b.ps
psxy -R -Jx -O -K -SvS0.005i/0.04i/0.03i -N -G0 << EOF >> GMT_Defaults_1b.ps
0 0.1 -0.4 0.1
0.25 0 0.25 -0.4
2.3 0.1 2.3 0.9
EOF
psxy -R -Jx -O -K -Svs0.005i/0.04i/0.03i -N -G0 << EOF >> GMT_Defaults_1b.ps
-0.2 1.15 -0.03 1
2.35 -0.2 2.0 -0.05
2.35 1.2 1.95 1.1
1.95 1.4 1.4 1.4
1.05 0.5 1.65 0.75
3.25 0.25 3.6 0.25
0.37 1.8 0.15 1.6
1.25 1.8 1 1.6
EOF
psxy -R -Jx -O -W0.25tap -X-0.5 -Y-0.5 -M << EOF >> GMT_Defaults_1b.ps
>
0.1	0.1
0.1	0.6
>
0.1	0.1
0.75	0.1
EOF
