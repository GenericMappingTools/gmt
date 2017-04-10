#!/bin/bash
#       $Id$
#
# Check GMT4-style vector symbols

ps=oldvec.ps

gmt gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
gmt psxyz -R0/6/-1/1 -Jx1i -p135/35 -P -K -W2p -Gred -S -B0g1 -B+t"GMT5 Vectors" << EOF > $ps
1	0	0	45	1.5i	v18p+jc+b+e+p4p,blue+a45
3	0	0	45	1.5i	v18p+jc+e+p-+a45
5	0	0	45	1.5i	v18p+jc+e+a45
EOF
gmt psxyz -R -J -O -p -W1p,blue -Gred -S -B0g1 -B+t"GMT4 Vectors" -Y3.5i << EOF >> $ps
1	0	0	45	1.5i	vB4p/18p/7.5p
3	0	0	45	1.5i	vb4p/18p/7.5p
5	0	0	45	1.5i	vb4p/18p/7.5p
EOF
