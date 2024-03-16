#!/usr/bin/env bash
#
# Plot plot bar symbol for the man page

gmt begin GMT_base_symbols4
    gmt set GMT_THEME cookbook
cat << EOF > ttv.d
1	1	0
2	3	1
EOF
cat << EOF > tth.d
1	1	0
3	2	0.7
EOF
cat << EOF > ttm.d
0.2	1	1.8	0.4	1.1
0.3	2	0.8	0.2	0.8
EOF
    gmt makecpt -Cjet -T0/4/1
    gmt plot -R0/3.5/0/3.75 -JX2i/1i -Bag1 -BWS -Sb0.2i+b -Glightblue -W0.5p -X0.5i -Y1i ttv.d --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5
    gmt plot -Bag1 -BWS -SB0.2i+b -Glightred -W0.5p -X2.5i tth.d --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5
    gmt plot -Bag1 -BWS -SB0.2i+i4 -C -W0.5p -X2.5i ttm.d --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5
    gmt plot -Bag1 -BWS -SB0.2i+i4+s25 -C -W0.25p -X2.5i ttm.d --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5
gmt end show
