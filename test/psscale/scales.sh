#!/usr/bin/env bash
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
gmt psscale -Dx3.25i/0i+w6i/0.25i+h+jTC -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red -P -K > $ps
gmt psscale -Dx3.25i/0i+w6i/0.25i+h+jTC -Ct.cpt  --MAP_FRAME_PEN=thicker,red -O -K -Y1i >> $ps
gmt psscale -Dx3.25i/0i+w6i/0.25i+h+jTC -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -Y1i >> $ps
gmt psscale -Dx3.25i/0i+w6i/0.25i+h+jTC -Ct.cpt  --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -Y1i >> $ps
# Vertical
gmt psscale -Dx0.5i/2i+w4i/0.25i+jML -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red -O -K -Y1i >> $ps
gmt psscale -Dx0.5i/2i+w4i/0.25i+jML -Ct.cpt  --MAP_FRAME_PEN=thicker,red -O -K -X1.5i >> $ps
gmt psscale -Dx0.5i/2i+w4i/0.25i+jML -Ct.cpt -Ba1 --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -K -X1.5i >> $ps
gmt psscale -Dx0.5i/2i+w4i/0.25i+jML -Ct.cpt  --MAP_FRAME_PEN=thicker,red --FONT_ANNOT_PRIMARY=12p,Helvetica,blue -O -X1.5i >> $ps
