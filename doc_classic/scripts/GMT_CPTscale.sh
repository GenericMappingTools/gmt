#!/bin/bash
#	$Id$
#
ps=GMT_CPTscale.ps
gmt psxy -R0/6/0/6 -Jx1i -P -K -W0.25p << EOF > $ps
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
gmt psscale -Cglobe -Baf -Dx3i/1.5i+w2.8i/0.15i+jCM -O -K -W0.001 >> $ps
gmt makecpt -Cglobe -T-500/3000 > t.cpt
gmt psscale -Ct.cpt -Baf -Dx5i/1.5i+w2.0i/0.15i+jLM -O -K -W0.001 >> $ps
gmt makecpt -Cglobe -G-3000/5000 -T-500/3000 > t.cpt
gmt psscale -Ct.cpt -Baf -Dx1i/1.5i+w2.0i/0.15i+jRM+ma -O -K -W0.001 >> $ps
gmt pstext -R -J -O -N -F+f14p+j << EOF >> $ps
0	0	LB	Scale a subset (via @%1%-G@%%)
6	0	RB	Scale entire range
3	3.1	CB	Master CPT
1	3.1	CB	New CPT v1
5	3.1	CB	New CPT v2
EOF
