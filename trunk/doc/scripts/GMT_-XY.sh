#!/bin/bash
#	$Id: GMT_-XY.sh,v 1.5 2011-02-28 00:58:00 remko Exp $
#
. functions.sh

psxy -R0/1.5/0/1.7 -Jx1 -P -B0 -K -Sv0.005/0.035/0.025 -Gblack << EOF > GMT_-XY.ps
0.2	0.2	0	1.1
0.2	0.2	90	1.4
EOF
psxy -R -J -O -K -m -Wthinnest,- << EOF >> GMT_-XY.ps
>
0	0.2
0.2	0.2
>
0.2	0
0.2	0.2
EOF
pstext -R -J -O -N << EOF >> GMT_-XY.ps
0.2	-0.05	10	0	1	TC	xoff
-0.05	0.2	10	0	1	RM	yoff
1.3	0.25	9	0	1	BL	x
0.25	1.6	9	0	1	ML	y
EOF
