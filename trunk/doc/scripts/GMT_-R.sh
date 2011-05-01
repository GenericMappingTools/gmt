#!/bin/bash
#	$Id: GMT_-R.sh,v 1.13 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

gmtset MAP_FRAME_TYPE PLAIN FONT_ANNOT_PRIMARY 8p,Helvetica MAP_TICK_LENGTH 0.05i \
	PS_CHAR_ENCODING ISOLatin1+
pscoast -R-90/-70/18/35.819 -JM2i -P -Dl -Glightgray -Wthinnest -Ba10g5WSEn -K > GMT_-R.ps
pstext -R0/2/-0.5/2 -Jx1i -O -K -N -Y-0.5 -F+f9p,Helvetica-Oblique+jCT << EOF >> GMT_-R.ps
1	-0.375	@%0%a)@%% @%1%\035R@%%xmin/xmax/ymin/ymax
EOF
psxy -R -J -O -K -N -Sc0.1 -Wthinner << EOF >> GMT_-R.ps
0	1
1	0
1	2
2	1
EOF
psxy -R -J -O -K -N -Wthinner << EOF >> GMT_-R.ps
>
0	1
0.675	-0.35
>
1	0
1.28	-0.35
>
1	2
1.67	-0.35
>
2	1
1.05	-0.35
EOF
#
pscoast -R-90/20/-65.5327/29.4248r -JOc280/20/22/69/2 -O -Dl -Glightgray -Wthinnest -Ba10g5WSEn -K -X2.75 -Y0.5 >> GMT_-R.ps
pstext -R0/2/-0.5/2 -Jx1i -O -K -N -Y-0.5 -F+f9p,Helvetica-Oblique+jCT << EOF >> GMT_-R.ps
1	-0.375	@%0%b)@%% @%1%\035R@%%xlleft/ylleft/xuright/yuright @%1%r@%%
EOF
psxy -R -J -O -K -N -Sc0.1 -Wthinner << EOF >> GMT_-R.ps
0	0
2	2
EOF
psxy -R -J -O -N -Wthinner << EOF >> GMT_-R.ps
>
0	0
0.56	-0.35
>
0	0
0.87	-0.35
>
2	2
1.2	-0.35
>
2	2
1.63	-0.35
EOF

