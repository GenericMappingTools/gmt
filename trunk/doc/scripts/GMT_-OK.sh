#!/bin/sh
#	$Id: GMT_-OK.sh,v 1.2 2003-02-06 18:18:57 pwessel Exp $
#

pstext -R0/2.7/0/2 -Jx1 -P -K -N << EOF > GMT_-OK.ps
0.5	1.8	10	0	1	CM	HEADER
0.5	1.35	10	0	1	CM	BODY@-1@-
0.5	0.55	10	0	1	CM	BODY@-n@-
0.5	0.1	10	0	1	CM	TRAILER
1.15	1.8	9	0	0	LM	@%1%\261O@%% ommits the header.
1.15	1.025	9	0	0	LM	2nd through n-1'th overlays
1.15	0.875	9	0	0	LM	require both @%1%\261O@%% and @%1%\261K@%%.
1.15	0.1	9	0	0	LM	@%1%\261K@%% ommits the trailer.
EOF
psxy -R -Jx -O -K -W1t2_2:0p << EOF >> GMT_-OK.ps
0.5	0.8
0.5	1.1
EOF
psxy -R -Jx -O -M -L -N -W1p << EOF >> GMT_-OK.ps
>
0	0
1	0
1	0.2
0	0.2
>
0	0.3
1	0.3
1	0.8
0	0.8
>
0	1.1
1	1.1
1	1.6
0	1.6
>
0	1.7
1	1.7
1	1.9
0	1.9
EOF
