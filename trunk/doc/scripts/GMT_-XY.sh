#!/bin/sh
#	$Id: GMT_-XY.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psxy -R0/1.5/0/1.7 -Jx1 -P -B0 -K -Sv0.005/0.035/0.025 -G0 << EOF > GMT_-XY.ps
0.2	0.2	0	1.1
0.2	0.2	90	1.4
EOF
psxy -R -Jx -O -K -M -W0.25tap << EOF >> GMT_-XY.ps
>
0	0.2
0.2	0.2
>
0.2	0
0.2	0.2
EOF
pstext -R -Jx -O -N << EOF >> GMT_-XY.ps
0.2	-0.05	10	0	1	TC	xoff
-0.05	0.2	10	0	1	RM	yoff
1.3	0.25	9	0	1	BL	x
0.25	1.6	9	0	1	ML	y
EOF
