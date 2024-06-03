#!/usr/bin/env bash
gmt begin GMT_CPTscale
	gmt plot -R0/6/0/6 -Jx1i -W0.25p << EOF
> Normal scaling of whole CPT
3	2.9
5	2.5
>
3	0.1
5	0.5
> Truncated with -G
3	2.2
1	2.5
>
3	1.08
1	0.5
> Dash the hinges -W0.25p,-
1	0.785
3	1.5
>
3	1.5
5	0.785
EOF
	gmt colorbar -Cglobe -B -Dx3i/1.5i+w2.8i/0.15i+jCM -W0.001
	gmt makecpt -Cglobe -T-500/3000
	gmt colorbar -C -B -Dx5i/1.5i+w2.0i/0.15i+jLM -W0.001
	gmt makecpt -Cglobe -G-3000/5000 -T-500/3000
	gmt colorbar -C -B -Dx1i/1.5i+w2.0i/0.15i+jRM+ma -W0.001
	gmt text -N -F+f14p+j << EOF
0	0	LB	Scale a subset (via @%1%-G@%%)
6	0	RB	Scale entire range
3	3.1	CB	Master CPT
1	3.1	CB	New CPT v1
5	3.1	CB	New CPT v2
EOF
gmt end show
