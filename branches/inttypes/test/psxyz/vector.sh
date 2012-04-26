#!/bin/bash
#       $Id$
#
# Check vector symbols

ps=vector.ps

psbasemap -R0/6/0/3 -Jx1i -P -B1g1WSne -K -Xc -p155/35 > $ps
gmtset MAP_VECTOR_SHAPE 0.5
# Center justified vectors
psxyz -R -J -O -K -W1p -Gred -S -p155/35 << EOF >> $ps
0.5	0.5	0	30	1i	v0.2i+jc
1.5	0.5	0	30	1i	v0.2i+jc+b
2.5	0.5	0	30	1i	v0.2i+jc+e+p-
3.5	0.5	0	30	1i	v0.2i+jc+b+e+p1p,blue
4.5	0.5	0	30	1i	v0.2i+jc+b+l
5.5	0.5	0	30	1i	v0.2i+jc+e+r
EOF
# Beginning justified vectors
psxyz -R -J -O -K -W1p -Gyellow -S -p155/35 << EOF >> $ps
0.1	1.2	0	30	1i	v0.2i+jb
1.1	1.2	0	30	1i	v0.2i+jb+b
2.1	1.2	0	30	1i	v0.2i+jb+e+gorange
3.1	1.2	0	30	1i	v0.2i+jb+b+e+g-
4.1	1.2	0	30	1i	v0.2i+jb+b+l
5.1	1.2	0	30	1i	v0.2i+jb+e+r
EOF
# End justified vectors
psxyz -R -J -O -K -W1p -S -p155/35 << EOF >> $ps
0.9	2.8	0	30	1i	v0.2i+je
1.9	2.8	0	30	1i	v0.2i+je+b
2.9	2.8	0	30	1i	v0.2i+je+e
3.9	2.8	0	30	1i	v0.2i+je+b+e
4.9	2.8	0	30	1i	v0.2i+je+b+l
5.9	2.8	0	30	1i	v0.2i+je+e+r
EOF
# Then with -SV and Mercator
gmtset MAP_VECTOR_SHAPE 1
psbasemap -R0/6/0/3 -Jm1i -P -B1g1WSne -O -K -Y4i -p155/35 >> $ps
# Center justified vectors
psxyz -R -J -O -K -W1p -Gred -S -p155/35 << EOF >> $ps
0.5	0.5	0	60	1i	V0.2i+jc
1.5	0.5	0	60	1i	V0.2i+jc+b
2.5	0.5	0	60	1i	V0.2i+jc+e+p-
3.5	0.5	0	60	1i	V0.2i+jc+b+e+p1p,blue
4.5	0.5	0	60	1i	V0.2i+jc+b+l
5.5	0.5	0	60	1i	V0.2i+jc+e+r
EOF
# Beginning justified vectors
psxyz -R -J -O -K -W1p -Gyellow -S -p155/35 << EOF >> $ps
0.1	1.2	0	60	1i	V0.2i+jb
1.1	1.2	0	60	1i	V0.2i+jb+b
2.1	1.2	0	60	1i	V0.2i+jb+e+gorange
3.1	1.2	0	60	1i	V0.2i+jb+b+e+g-
4.1	1.2	0	60	1i	V0.2i+jb+b+l
5.1	1.2	0	60	1i	V0.2i+jb+e+r
EOF
# End justified vectors
psxyz -R -J -O -W1p -S -p155/35 << EOF >> $ps
0.9	2.8	0	60	1i	V0.2i+je
1.9	2.8	0	60	1i	V0.2i+je+b
2.9	2.8	0	60	1i	V0.2i+je+e
3.9	2.8	0	60	1i	V0.2i+je+b+e
4.9	2.8	0	60	1i	V0.2i+je+b+l
5.9	2.8	0	60	1i	V0.2i+je+e+r
EOF

