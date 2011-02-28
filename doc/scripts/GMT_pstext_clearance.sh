#!/bin/bash
#	$Id: GMT_pstext_clearance.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

gmtset GLOBAL_X_SCALE 0.8 GLOBAL_Y_SCALE 0.8
pstext -R0/3/-0.1/1.5 -Jx1 -P -K -C0.2/0.2 -Wothick << EOF > GMT_pstext_clearance.ps
1.5	0.5	36	0	1	CM	My Text
EOF
pstext -R -J -O -K -C0/0 -Wothin,- << EOF >> GMT_pstext_clearance.ps
1.5	0.5	36	0	1	CM	My Text
EOF
pstext -R -J -O -K << EOF >> GMT_pstext_clearance.ps
2	0.75	9	0	0	LM	@~D@~y
2.53	0.65	9	0	0	CB	@~D@~x
EOF
psxy -R -J -O -m << EOF >> GMT_pstext_clearance.ps
>
1.95	0.63
1.95	0.82
>
2.44	0.6
2.64	0.6
EOF
