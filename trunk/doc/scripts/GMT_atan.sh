#!/bin/sh
#	$Id: GMT_atan.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

grdgradient -A45 ../../tutorial/us.grd -N -M -Gt.grd
grd2xyz -Z t.grd > $$
pshistogram $$ -R-0.75/0.75/0/20 -JX1.5/1 -B0.5/5f5WSne -W0.01 -P -K -G0 -Z1 > GMT_atan.ps

pstext -R -JX -O -K << EOF >> GMT_atan.ps
-0.7	17	9	0	0	LB	Raw 
-0.7	15	9	0	0	LB	slopes
EOF
gmtmath -T-5/5/0.01 T ATAN PI DIV 2 MUL = > $$
psxy $$ -R-5/5/-1/1 -JX1.5/1 -B2f1g1/1f0.5g0.25WSne -O -K -W1p -X1.85 >> GMT_atan.ps
psxy -R -JX -O -K -Sv0.004/0.03/0.02 -G0 << EOF >> GMT_atan.ps
3	0.8	180	0.45
3	0.8	-90	0.4
EOF
psxy -R -JX -O -K -M -W0.25p << EOF >> GMT_atan.ps
>
-5	0
5	0
>
0	-1
0	1
EOF
grdgradient -A45 ../../tutorial/us.grd -Nt -M -Gtt.grd
grd2xyz -Z tt.grd > $$
pshistogram $$ -R-0.75/0.75/0/5 -JX1.5/1 -B0.5/2f1WSne -W0.01 -O  -K -G0 -X1.85 -Z1 >> GMT_atan.ps
pstext -R -JX -O << EOF >> GMT_atan.ps
-0.7	4.3	9	0	0	LB	tan@+-1@+ 
-0.7	3.7	9	0	0	LB	transformed
EOF
\rm -f t.grd tt.grd $$
