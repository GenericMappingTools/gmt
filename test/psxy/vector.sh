#!/usr/bin/env bash
#
# Check vector symbols

ps=vector.ps

gmt psbasemap -R0/6/0/3 -Jx1i -P -B1g1 -BWSne -K -Xc > $ps
gmt set MAP_VECTOR_SHAPE 0.5
# Center justified vectors
gmt psxy -R -J -O -K -W1p -Gred -S << EOF >> $ps
0.5	0.5	30	1i	v0.2i+jc
1.5	0.5	30	1i	v0.2i+jc+b
2.5	0.5	30	1i	v0.2i+jc+e+p-
3.5	0.5	30	1i	v0.2i+jc+b+e+p1p,blue
4.5	0.5	30	1i	v0.2i+jc+bl
5.5	0.5	30	1i	v0.2i+jc+er
EOF
# Beginning justified vectors
gmt psxy -R -J -O -K -W1p -Gyellow -S << EOF >> $ps
0.1	1.2	30	1i	v0.2i+jb
1.1	1.2	30	1i	v0.2i+jb+b
2.1	1.2	30	1i	v0.2i+jb+e+gorange
3.1	1.2	30	1i	v0.2i+jb+b+e+g-
4.1	1.2	30	1i	v0.2i+jb+bl
5.1	1.2	30	1i	v0.2i+jb+er
EOF
# End justified vectors
gmt psxy -R -J -O -K -W1p -S << EOF >> $ps
0.9	2.8	30	1i	v0.2i+je
1.9	2.8	30	1i	v0.2i+je+b
2.9	2.8	30	1i	v0.2i+je+e
3.9	2.8	30	1i	v0.2i+je+b+e
4.9	2.8	30	1i	v0.2i+je+bl
5.9	2.8	30	1i	v0.2i+je+er
EOF
# Then with -SV and Mercator
gmt set MAP_VECTOR_SHAPE 1
gmt psbasemap -R0/6/0/3 -Jm1i -P -B1g1 -BWSne -O -K -Y4i >> $ps
# Center justified vectors
gmt psxy -R -J -O -K -W1p -Gred -S << EOF >> $ps
0.5	0.5	60	1i	V0.2i+jc
1.5	0.5	60	1i	V0.2i+jc+b
2.5	0.5	60	1i	V0.2i+jc+e+p-
3.5	0.5	60	1i	V0.2i+jc+b+e+p1p,blue
4.5	0.5	60	1i	V0.2i+jc+bl
5.5	0.5	60	1i	V0.2i+jc+er
EOF
# Beginning justified vectors
gmt psxy -R -J -O -K -W1p -Gyellow -S << EOF >> $ps
0.1	1.2	60	1i	V0.2i+jb
1.1	1.2	60	1i	V0.2i+jb+b
2.1	1.2	60	1i	V0.2i+jb+e+gorange
3.1	1.2	60	1i	V0.2i+jb+b+e+g-
4.1	1.2	60	1i	V0.2i+jb+bl
5.1	1.2	60	1i	V0.2i+jb+er
EOF
# End justified vectors
gmt psxy -R -J -O -W1p -S << EOF >> $ps
0.9	2.8	60	1i	V0.2i+je
1.9	2.8	60	1i	V0.2i+je+b
2.9	2.8	60	1i	V0.2i+je+e
3.9	2.8	60	1i	V0.2i+je+b+e
4.9	2.8	60	1i	V0.2i+je+bl
5.9	2.8	60	1i	V0.2i+je+er
EOF

