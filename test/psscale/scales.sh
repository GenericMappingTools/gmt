#!/bin/bash
# Test psscale when pen and font changes are applied simultaneously
ps=scales.ps
cat << EOF >t.cpt
0	gray5	1	gray5
1	gray15	3	gray15
3	gray35	4	gray35
4	gray45	5	gray45
5	gray55	6	gray55
6	gray65	7	gray65
7	gray75	8	gray75
8	gray95	10	gray95
B	black
F	white
N	127.5
EOF
# Horizontal
gmt psscale -D3.25i/0i/6i/0.25ih -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red -P -K > $ps
gmt psscale -D3.25i/0i/6i/0.25ih -Ct.cpt  --MAP_FRAME_PEN=thicker,red -O -K -Y1i >> $ps
gmt psscale -D3.25i/0i/6i/0.25ih -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -Y1i >> $ps
gmt psscale -D3.25i/0i/6i/0.25ih -Ct.cpt  --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -Y1i >> $ps
# Vertical
gmt psscale -D0.5i/2i/4i/0.25i -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red -O -K -Y1i >> $ps
gmt psscale -D0.5i/2i/4i/0.25i -Ct.cpt  --MAP_FRAME_PEN=thicker,red -O -K -X1.5i >> $ps
gmt psscale -D0.5i/2i/4i/0.25i -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -X1.5i >> $ps
gmt psscale -D0.5i/2i/4i/0.25i -Ct.cpt  --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -X1.5i >> $ps
