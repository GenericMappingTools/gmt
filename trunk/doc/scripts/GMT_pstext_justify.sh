#!/bin/bash
#	$Id: GMT_pstext_justify.sh,v 1.6 2011-02-28 00:58:01 remko Exp $
#
. functions.sh

B=0.2
M=0.38
T=0.56
gmtset GLOBAL_X_SCALE 0.8 GLOBAL_Y_SCALE 0.8
pstext -R0/3/0/1.5 -Jx1i -P -K -N -C0/0 -Wothin,- << EOF > GMT_pstext_justify.ps
0	0	36	0	1	LB	My Text
EOF
psxy -R -J -O -K -m -N -X-0.1 -Y-0.2 << EOF >> GMT_pstext_justify.ps
>
0.05	$B
2.04	$B
>
0.05	$M
2.04	$M
>
0.05	$T
2.04	$T
>
0.135	0
0.135	0.65
>
1	0
1	0.65
>
1.945	0
1.945	0.65
EOF
psxy -R -J -O -K -m -N -Wthinner << EOF >> GMT_pstext_justify.ps
>
0.7	-0.1
0.135	$M
>
1.3	-0.1
1.945	$T
EOF
pstext -R -J -O -K -N << EOF >> GMT_pstext_justify.ps
0.135	0.69	8	0	0	CB	L (Left)
1	0.69	8	0	0	CB	C (Center)
1.945	0.69	8	0	0	CB	R (Right)
2.07	$T	8	0	0	LM	T (Top)
2.07	$M	8	0	0	LM	M (Middle)
2.07	$B	8	0	0	LM	B (Bottom)
0.6	-0.05	8	0	0	LM	LM
1.37	-0.05	8	0	0	RM	TR
EOF
psxy -R -J -O -Sc0.05 << EOF >> GMT_pstext_justify.ps
0.135	$B
0.135	$M
0.135	$T
1	$B
1	$M
1	$T
1.945	$B
1.945	$M
1.945	$T
EOF
