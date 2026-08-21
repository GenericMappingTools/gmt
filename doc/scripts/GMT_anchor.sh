#!/usr/bin/env bash
gmt begin GMT_anchor
	gmt basemap -R0/1/0/1 -JX5i/2i -Ba1f0.5 -BwSnE+gbisque
	gmt inset begin -DjTL+o0.7i/0.5i+w1.5i/0.75i -F+glightgreen+p1p
	gmt inset end
	gmt plot -W0.25p,- << EOF
0	0.75
0.14	0.75
0.14	1
EOF
	gmt plot -W2p << EOF
0	1
0.14	0.75
EOF
	echo 0 1 | gmt plot  -Ss0.2i -Gred -W0.25p -N
	echo 0.14 0.75 | gmt plot  -Ss0.2i -Gorange -W0.25p -N
	gmt plot -Sc0.075i -Gblue -W0.25p -N << EOF
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
	gmt plot -Sc0.075i -Gcyan -W0.25p -N << EOF
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
	gmt text -F+f14p,Helvetica-Bold+j -Dj0.25i+v0.25p -N << EOF
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
	echo 0 0.75 0 1 | gmt plot -N -Sv0.2i+bt+et+s -W1p -D-0.2i/0
	echo 0 1 0.14 1 | gmt plot -N -Sv0.2i+bt+et+s -W1p -D0/0.2i
	echo 0.07 1 dx  | gmt text -N -F+f12p,Times-Italic+jCM -Gwhite -D0/0.2i
	echo 0 0.875 dy | gmt text -N -F+f12p,Times-Italic+jCM -Gwhite -D-0.2i/0
gmt end show
