#!/bin/sh
#	$Id: GMT_nearneighbor.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psxy -R0/2/0/2 -Jx1 -B0g0.25 -Sc0.025 -G0 -P -K << EOF > GMT_nearneighbor.ps
0.04	1.8
0.3	0.3
0.31	0.7
0.65	1.88
0.9	0.44
0.88	0.2
1.3	0.8
1.72	1.1
1.33	1.59
1.8	1.9
1.82	0.5
1.6	1.3
#
0.5	0.9
0.52	1.52
0.4	1.2
1.15	1.3
1	0.88
EOF
psxy -R -Jx -O -K -Sc1 -W1p << EOF >> GMT_nearneighbor.ps
0.75	1.25
EOF
psxy -R -Jx -O -K -M -W0.5p << EOF >> GMT_nearneighbor.ps
>
0.25	1.25
1.25	1.25
>
0.75	0.75
0.75	1.75
EOF
psxy -R -Jx -O -K -M -W0.25p << EOF >> GMT_nearneighbor.ps
>
0.75	1.25
0.5	0.9
>
0.75	1.25
0.52	1.52
>
0.75	1.25
0.4	1.2
>
0.75	1.25
1.15	1.3
>
0.75	1.25
1	0.88
EOF
psxy -R -Jx -O -K -M -W0.5top << EOF >> GMT_nearneighbor.ps
0.75	1.25
1.10355	1.60355
EOF
pstext -R -Jx -O << EOF >> GMT_nearneighbor.ps
1	1.4	8	0	2	BL	R
1.0	1	8	0	2	BL	r@-i@-
EOF
