#!/bin/bash
#	$Id: GMT_-OK.sh,v 1.12 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

psxy -R0/2.7/0/2.5 -Jx1i -K -L -N -P << EOF > GMT_-OK.ps
> -Glightyellow
0	1.2
1	1.2
1	2.1
0	2.1
> -Glightred
0	1.4
1	1.4
1	1.2
0	1.2
> -Glightgreen
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick -G-
0	1.2
1	1.2
1	2.1
0	2.1
EOF
pstext -R -J -O -K -N -F+f << EOF >> GMT_-OK.ps
0.5	2.3	9p,Helvetica		1-part PostScript file
0.5	2.0	10p,Helvetica-Bold	HEADER
0.5	1.65	10p,Helvetica-Bold	BODY@-1@-
0.5	1.3	10p,Helvetica-Bold	TRAILER
EOF

psxy -R -J -X1.3i -O -K -L -N << EOF >> GMT_-OK.ps
> -Glightyellow
0	0.6
1	0.6
1	1.3
0	1.3
> -Glightred
0	0.8
1	0.8
1	0.6
0	0.6
> -Glightyellow
0	1.4
1	1.4
1	2.1
0	2.1
> -Glightgreen
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick -G-
0	0.6
1	0.6
1	1.3
0	1.3
> -Wthick -G-
0	1.4
1	1.4
1	2.1
0	2.1
> -Wthick
0.5	1.3
0.5	1.4
EOF
pstext -R -J -O -K -N -F+f+j << EOF >> GMT_-OK.ps
0.5	2.3	9p,Helvetica		CM	2-part PostScript file
0.5	2.0	10p,Helvetica-Bold	CM	HEADER
0.5	1.65	10p,Helvetica-Bold	CM	BODY@-1@-
1.1	1.5	9p,Helvetica		LM	@%1%\035K@%% omits trailer
1.1	1.2	9p,Helvetica		LM	@%1%\035O@%% omits header
0.5	1.05	10p,Helvetica-Bold	CM	BODY@-2@-
0.5	0.7	10p,Helvetica-Bold	CM	TRAILER
EOF

psxy -R -J -X2.3i -O -K -L -N << EOF >> GMT_-OK.ps
> -Glightyellow
0	0.0
1	0.0
1	0.7
0	0.7
> -Glightred
0	0.2
1	0.2
1	0.0
0	0.0
> -Glightyellow
0	1.4
1	1.4
1	2.1
0	2.1
> -Glightgreen
0	1.9
1	1.9
1	2.1
0	2.1
> -Wthick -G-
0	0.0
1	0.0
1	0.7
0	0.7
> -Wthick -Glightyellow
0	0.8
1	0.8
1	1.3
0	1.3
> -Wthick -G-
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
pstext -R -J -O -N -F+f+j << EOF >> GMT_-OK.ps
0.5	2.3	9p,Helvetica		CM	n-part PostScript file
0.5	2.0	10p,Helvetica-Bold	CM	HEADER
0.5	1.65	10p,Helvetica-Bold	CM	BODY@-1@-
1.1	1.5	9p,Helvetica		LM	@%1%\035K@%% omits trailer
0.5	1.05	10p,Helvetica-Bold	CM	BODY@-i@-
1.1	1.25	9p,Helvetica		LM	2nd through n-1'th
1.1	1.1	9p,Helvetica		LM	overlays require
1.1	0.95	9p,Helvetica		LM	both @%1%\035O@%% and @%1%\035K@%%
1.1	0.6	9p,Helvetica		LM	@%1%\035O@%% omits header
0.5	0.45	10p,Helvetica-Bold	CM	BODY@-n@-
0.5	0.1	10p,Helvetica-Bold	CM	TRAILER
EOF
