#!/usr/bin/env bash
gmt begin GMT_nearneighbor
gmt plot -R0/2/0/2 -Jx1i -Sc1i -Wthick -Glightgreen@70 -Bg0.25 << EOF
0.75	1.25
EOF
gmt plot -Sc0.05i -Gblack << EOF
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
gmt plot -Wthinner << EOF
>
0.25	1.25
1.25	1.25
>
0.75	0.75
0.75	1.75
EOF
gmt plot -Wthinnest << EOF
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
gmt plot -W0.5p,.- << EOF
0.75	1.25
1.10355	1.60355
EOF
gmt text -F+f8p,Helvetica-Oblique+jBL << EOF
1	1.4	R
1.0	1	r@-i@-
EOF
gmt end show
