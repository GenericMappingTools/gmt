#!/usr/bin/env bash
gmt begin GMT_-R
	gmt set MAP_FRAME_TYPE PLAIN FONT_ANNOT_PRIMARY 8p,Helvetica MAP_TICK_LENGTH_PRIMARY 0.05i \
		PS_CHAR_ENCODING ISOLatin1+
	gmt coast -R-90/-70/18/35.819 -JM2i -Dl -Glightbrown -Wthinnest -Ba10g5 -BWsEN
	gmt text -R0/2/-0.5/2 -Jx1i -N -Y-0.5i -F+f9p,Helvetica-Oblique+jCT << EOF
1	-0.375	@%0%a)@%% @%1%\035R@%%xmin/xmax/ymin/ymax
EOF
	gmt plot -N -Sc0.1i -Wthinner << EOF
0	1
1	0
1	2
2	1
EOF
	gmt plot -N -Wthinner << EOF
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
	gmt coast -R-90/20/-65.5327/29.4248+r -JOc280/20/22/69/2i -Dl -Glightbrown -Wthinnest -Ba10g5 -BWsEN -X2.75i -Y0.5i
	gmt text -R0/2/-0.5/2 -Jx1i -N -Y-0.5i -F+f9p,Helvetica-Oblique+jCT << EOF
1	-0.375	@%0%b)@%% @%1%\035R@%%xlleft/ylleft/xuright/yuright@%1%+r@%%
EOF
	gmt plot -N -Sc0.1i -Wthinner << EOF
0	0
2	2
EOF
	gmt plot -N -Wthinner << EOF
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
gmt end show
