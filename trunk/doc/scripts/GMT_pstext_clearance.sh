#!/bin/bash
#	$Id$
#
. ./functions.sh

gmtset PS_SCALE_X 0.8 PS_SCALE_Y 0.8
pstext -R0/3/-0.1/1.5 -Jx1i -P -K -C0.2 -TO -Wthick -F+f36p,Helvetica-Bold << EOF > GMT_pstext_clearance.ps
1.5	0.5	My Text
EOF
pstext -R -J -O -K -C0 -Wthin,- -F+f36p,Helvetica-Bold << EOF >> GMT_pstext_clearance.ps
1.5	0.5	My Text
EOF
pstext -R -J -O -K -F+f9p+j << EOF >> GMT_pstext_clearance.ps
2.00	0.80	LM	@~D@~y
2.52	0.65	CB	@~D@~x
0.56	0.75	LB	r
EOF
psxy -R -J -O << EOF >> GMT_pstext_clearance.ps
>
1.95	0.69
1.95	0.89
>
2.42	0.60
2.62	0.60
>
0.59	0.69
0.46	0.82
EOF
