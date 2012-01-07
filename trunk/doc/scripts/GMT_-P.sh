#!/bin/bash
#	$Id$
#
. ./functions.sh

psxy -R0/2.5/0/1.7 -Jx1i -P -K -Glightyellow -Wthinner << EOF >| GMT_-P.ps
>
0	0
1	0
1	1.3
0	1.3
>
1.25	0
2.25	0
2.25	1.3
1.25	1.3
EOF
pstext -R -J -O -K -N -F+f+a+j << EOF >> GMT_-P.ps
1.125	1.525	9p,Helvetica		0	CB	leading
1.125	1.4	9p,Helvetica		0	CB	paper edge
0.5	0.65	10p,Helvetica-Bold	0	CM	\035P
1.75	0.65	10p,Helvetica-Bold	0	CM	Default
0.8	0.15	9p,Helvetica-Oblique	0	BL	x
0.15	1.1	9p,Helvetica-Oblique	0	BL	y
2.1	1.1	9p,Helvetica-Oblique	90	BL	x
1.45	0.15	9p,Helvetica-Oblique	90	BL	y
EOF
psxy -R -J -O -K -Sv0.2i+e+a50 -W8p,lightred -Glightred << EOF >> GMT_-P.ps
0.5	1.35	90	0.3
1.75	1.35	90	0.3
EOF
psxy -R -J -O -Sv0.02i+e -W0.5p -Gblack << EOF >> GMT_-P.ps
0.1	0.1	0	0.7
0.1	0.1	90	1
2.15	0.1	180	0.7
2.15	0.1	90	1
EOF
