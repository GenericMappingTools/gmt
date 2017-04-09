#!/bin/bash
#       $Id$
#
# Check GMT4-style vector symbols

ps=oldvec.ps

gmt gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
gmt psxy -R0/6/-1/1 -Jx1i -P -K -W1p -Gred -S -B0g1 << EOF > $ps
1	0	45	1.5i	v0.2i+jc+b+e+p1p,blue
3	0	45	1.5i	v0.2i+jc+e+p-
5	0	45	1.5i	v0.2i+jc+b
EOF
gmt psxy -R -J -O -W1p -Gred -S -B0g1 -Y3.5i << EOF >> $ps
1	0	45	1.5i	vS0.04/0.25/0.2
3	0	45	1.5i	v0.04/0.25/0.2
5	0	45	1.5i	v0.04/0.25/0.2
EOF
