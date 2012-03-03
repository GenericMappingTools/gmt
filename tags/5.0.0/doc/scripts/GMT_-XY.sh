#!/bin/bash
#	$Id$
#
. ./functions.sh

psbasemap -R0/1.5/0/1.7 -Jx1i -P -K -B0+glightyellow >| GMT_-XY.ps
psxy -R -J -O -K -Sv5p+e -W0.5p -Gblack << EOF >> GMT_-XY.ps
0.2	0.2	0	1.1
0.2	0.2	90	1.4
EOF
psxy -R -J -O -K -Wthinnest,- << EOF >> GMT_-XY.ps
>
0	0.2
0.2	0.2
>
0.2	0
0.2	0.2
EOF
pstext -R -J -O -N --FONT=Helvetica-Bold -F+f+j << EOF >> GMT_-XY.ps
0.2	-0.05	10p	TC	xoff
-0.05	0.2	10p	RM	yoff
1.3	0.25	9p	BL	x
0.25	1.6	9p	ML	y
EOF
