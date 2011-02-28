#!/bin/bash
#	$Id: GMT_Defaults_1b.sh,v 1.10 2011-02-28 00:58:03 remko Exp $
#
. functions.sh
gmtset BASEMAP_TYPE plain PLOT_DEGREE_FORMAT ddd:mm:ssF GRID_CROSS_SIZE_PRIMARY 0i \
ANNOT_FONT_SIZE_PRIMARY +8p OBLIQUE_ANNOTATION 1
psbasemap -R-90/20/-55/25r -JOc-80/25.5/2/69/2.25i -Ba10f5g5 -P -K > GMT_Defaults_1b.ps
pstext -R0/2.25/0/2 -Jx1i -O -K -N << EOF >> GMT_Defaults_1b.ps
-0.05 0.15 7 0 1 RB X_ORIGIN
0.28 -0.2 7 0 1 LM Y_ORIGIN
-0.2 1.2 7 0 1 RB FRAME_PEN
2.4 -0.2 7 0 1 LT OBLIQUE_ANNOTATION
2.4 1.2 7 0 1 LB GRID_PEN_PRIMARY
2 1.4 7 0 1 LB ANNOT_MAX_ANGLE
2.35 0.5 7 0 1 LM ANNOT_MIN_SPACING
1 0.5 7 0 1 RT LINE_STEP
2.7 0.32 7 0 1 LM PAGE_COLOR
2.7 0.18 7 0 1 LM PAGE_MEDIA
0.4 1.9 7 0 1 LB ANNOT_FONT_PRIMARY
1.5 1.8 7 0 1 LB ANNOT_FONT_SIZE_PRIMARY
EOF
echo -0.4 -0.4 | psxy -R -J -O -K -Sc0.04 -Gblack -N >> GMT_Defaults_1b.ps
psxy -R -J -O -K -SvS0.005i/0.04i/0.03i -N -Gblack << EOF >> GMT_Defaults_1b.ps
0 0.1 -0.4 0.1
0.25 0 0.25 -0.4
2.3 0.1 2.3 0.9
EOF
psxy -R -J -O -K -Svs0.005i/0.04i/0.03i -N -Gblack << EOF >> GMT_Defaults_1b.ps
-0.2 1.15 -0.03 1
2.35 -0.2 2.0 -0.05
2.35 1.2 1.95 1.1
1.95 1.4 1.4 1.4
1.05 0.5 1.65 0.75
3.25 0.25 3.6 0.25
0.37 1.9 0.15 1.6
1.45 1.8 1 1.6
EOF
psxy -R -J -O -Wthinnest,- -X-0.5 -Y-0.5 -m << EOF >> GMT_Defaults_1b.ps
>
0.1	0.1
0.1	0.6
>
0.1	0.1
0.75	0.1
EOF
