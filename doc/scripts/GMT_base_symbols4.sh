#!/usr/bin/env bash
#
# Plot psxy bar symbol for the man page

ps=GMT_base_symbols4.ps

cat << EOF > ttv.d
1	1	0
2	3	1
EOF
cat << EOF > tth.d
1	1	0
3	2	0.7
EOF
gmt psxy -R0/3.5/0/3.75 -JX2i/1i -P -Bag1 -BWS -Sb0.2i+b -Glightblue -W0.5p -X1i -Y1i ttv.d -K --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5 > $ps
gmt psxy -R -J -O -Bag1 -BWS -SB0.2i+b -Glightred -W0.5p -X3i tth.d  --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5 >> $ps
