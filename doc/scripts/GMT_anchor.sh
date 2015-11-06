#!/bin/bash
#	$Id$
#
ps=GMT_anchor.ps
gmt psbasemap -R0/1/0/1 -JX5i/2i -Ba1f0.5 -BwSnE+gbisque -P -K -DjTL+o0.7i/0.5i+w1.5i/0.75i -F+glightgreen+p1p > $ps
gmt psxy -R -J -O -K -W0.25p,- << EOF >> $ps
0	0.75
0.14	0.75
0.14	1
EOF
gmt psxy -R -J -O -K -W2p << EOF >> $ps
0	1
0.14	0.75
EOF
echo 0 1 | gmt psxy  -R -J -O -K -Ss0.2i -Gred -W0.25p -N >> $ps
echo 0.14 0.75 | gmt psxy  -R -J -O -K -Ss0.2i -Gorange -W0.25p -N >> $ps
gmt psxy -R -J -O -K -Sc0.075i -Gblue -W0.25p -N << EOF >> $ps
0	0
0.5	0
1	0
0	0.5
0.5	0.5
1	0.5
0	1
0.5	1
1	1
EOF
gmt psxy -R -J -O -K -Sc0.075i -Gcyan -W0.25p -N << EOF >> $ps
0.14	0.375
0.29	0.375
0.44	0.375
0.14	0.5625
0.29	0.5625
0.44	0.5625
0.14	0.75
0.29	0.75
0.44	0.75
EOF
gmt pstext -R -J -O -K -F+f14p,Helvetica-Bold+j -Dj0.25iv0.25p -N << EOF >> $ps
0	0  	RT	LB
0.5	0  	CT	CB
1	0  	LT	RB
0	0.5	RM	LM
0.5	0.5	LM	CM
1	0.5	LM	RM
0	1  	RB	LT
0.5	1  	CB	CT
1	1  	LB	RT
EOF
echo 0 0.75 0 1 | gmt psxy -R -J -O -K -N -Sv0.2i+bt+et+s -W1p -D-0.2i/0 >> $ps
echo 0 1 0.14 1 | gmt psxy -R -J -O -K -N -Sv0.2i+bt+et+s -W1p -D0/0.2i >> $ps
echo 0.07 1 dx  | gmt pstext -N -R -J -O -K -F+f12p,Times-Italic+jCM -Gwhite -D0/0.2i >> $ps
echo 0 0.875 dy | gmt pstext -N -R -J -O -K -F+f12p,Times-Italic+jCM -Gwhite -D-0.2i/0 >> $ps
gmt psxy -R -J -O -T >> $ps
