#!/bin/sh
#	$Id: GMT_pstext_clearance.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset GLOBAL_X_SCALE 0.8 GLOBAL_Y_SCALE 0.8
pstext -R0/3/0/1.5 -Jx1 -P -K -C0.2/0.2 -Wo1p << EOF > GMT_pstext_clearance.ps
1.5	0.5	36	0	1	CM	My Text
EOF
pstext -R -Jx -O -K -C0/0 -Wo0.75t1_1:0p << EOF >> GMT_pstext_clearance.ps
1.5	0.5	36	0	1	CM	My Text
EOF
pstext -R -Jx -O -K << EOF >> GMT_pstext_clearance.ps
2	0.78	9	0	0	LM	@~D@~y
2.51	0.65	9	0	0	CB	@~D@~x
EOF
psxy -R -Jx -O -M << EOF >> GMT_pstext_clearance.ps
>
1.95	0.68
1.95	0.88
>
2.41	0.6
2.61	0.6
EOF
gmtset GLOBAL_X_SCALE 1 GLOBAL_Y_SCALE 1
