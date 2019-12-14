#!/usr/bin/env bash
gmt begin GMT_-XY
	gmt basemap -R0/1.5/0/1.7 -Jx1i -B0 -B+glightyellow
	gmt plot -Sv5p+e -W0.5p -Gblack << EOF
0.2	0.2	0	1.1
0.2	0.2	90	1.4
EOF
	gmt plot -Wthinnest,- << EOF
>
0	0.2
0.2	0.2
>
0.2	0
0.2	0.2
EOF
	gmt text -N --FONT=Helvetica-Bold -F+f+j << EOF
0.2	-0.05	10p	TC	xoff
-0.05	0.2	10p	RM	yoff
1.3	0.25	9p	BL	x
0.25	1.6	9p	ML	y
EOF
gmt end show
