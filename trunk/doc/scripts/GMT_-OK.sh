#!/bin/bash
#	$Id: GMT_-OK.sh,v 1.10 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

psxy -R0/2.7/0/2.5 -Jx1i -K -L -m -N -P << EOF > GMT_-OK.ps
> -G200
0	1.4
1	1.4
1	1.2
0	1.2
> -G200
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick
0	1.2
1	1.2
1	2.1
0	2.1
EOF
pstext -R -J -O -K -N << EOF >> GMT_-OK.ps
0.5	2.3	9	0	0	CM	1-part PostScript file
0.5	2.0	10	0	1	CM	HEADER
0.5	1.65	10	0	1	CM	BODY@-1@-
0.5	1.3	10	0	1	CM	TRAILER
EOF

psxy -R -J -X1.3i -O -K -L -m -N << EOF >> GMT_-OK.ps
> -G200
0	0.8
1	0.8
1	0.6
0	0.6
> -G200
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick
0	0.6
1	0.6
1	1.3
0	1.3
> -Wthick
0	1.4
1	1.4
1	2.1
0	2.1
> -Wthick
0.5	1.3
0.5	1.4
EOF
pstext -R -J -O -K -N << EOF >> GMT_-OK.ps
0.5	2.3	9	0	0	CM	2-part PostScript file
0.5	2.0	10	0	1	CM	HEADER
0.5	1.65	10	0	1	CM	BODY@-1@-
1.1	1.5	9	0	0	LM	@%1%\035K@%% omits trailer
1.1	1.2	9	0	0	LM	@%1%\035O@%% omits header
0.5	1.05	10	0	1	CM	BODY@-2@-
0.5	0.7	10	0	1	CM	TRAILER
EOF

psxy -R -J -X2.3i -O -K -m -L -N << EOF >> GMT_-OK.ps
> -G200
0	0.2
1	0.2
1	0.0
0	0.0
> -G200
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick
0	0.0
1	0.0
1	0.7
0	0.7
> -Wthick
0	0.8
1	0.8
1	1.3
0	1.3
> -Wthick
0	1.4
1	1.4
1	2.1
0	2.1
> -Wthick
0.5	1.3
0.5	1.4
> -Wthick
0.5	0.7
0.5	0.8
EOF
pstext -R -J -O -N << EOF >> GMT_-OK.ps
0.5	2.3	9	0	0	CM	n-part PostScript file
0.5	2.0	10	0	1	CM	HEADER
0.5	1.65	10	0	1	CM	BODY@-1@-
1.1	1.5	9	0	0	LM	@%1%\035K@%% omits trailer
0.5	1.05	10	0	1	CM	BODY@-i@-
1.1	1.25	9	0	0	LM	2nd through n-1'th
1.1	1.1	9	0	0	LM	overlays require
1.1	0.95	9	0	0	LM	both @%1%\035O@%% and @%1%\035K@%%
1.1	0.6	9	0	0	LM	@%1%\035O@%% omits header
0.5	0.45	10	0	1	CM	BODY@-n@-
0.5	0.1	10	0	1	CM	TRAILER
EOF
