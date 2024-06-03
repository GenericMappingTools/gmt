#!/usr/bin/env bash
gmt begin GMT_pstext_clearance
gmt text -R0/3/-0.1/1.5 -Jx1i -C0.2i+tO -Wthick -F+f36p,Helvetica-Bold << EOF
1.5	0.5	My Text
EOF
gmt text -C0 -Wthin,- -F+f36p,Helvetica-Bold << EOF
1.5	0.5	My Text
EOF
gmt text -F+f9p+j << EOF
2.00	0.80	LM	dy
2.52	0.65	CB	dx
0.56	0.75	LB	r
EOF
gmt plot << EOF
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
gmt end show
