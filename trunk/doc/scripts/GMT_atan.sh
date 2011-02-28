#!/bin/bash
#	$Id: GMT_atan.sh,v 1.10 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

trap 'rm -f $$.*; exit 1' 1 2 3 15

grdgradient -A45 ../../share/doc/gmt/tutorial/us.nc -N -M -G$$.t.nc
grd2xyz -Z $$.t.nc > $$.d
pshistogram $$.d -R-0.75/0.75/0/20 -JX1.5/1 -B0.5/5f5WSne -W0.01 -P -K -Gblack -Z1 > GMT_atan.ps

pstext -R -J -O -K << EOF >> GMT_atan.ps
-0.7	17	9	0	0	LB	Raw 
-0.7	15	9	0	0	LB	slopes
EOF
gmtmath -T-5/5/0.01 T ATAN PI DIV 2 MUL = > $$.d
psxy $$.d -R-5/5/-1/1 -JX1.5/1 -B2f1g1/1f0.5g0.25WSne -O -K -Wthick -X1.85 >> GMT_atan.ps
psxy -R -J -O -K -Sv0.004/0.03/0.02 -Gblack << EOF >> GMT_atan.ps
3	0.8	180	0.45
3	0	90	0.4
EOF
psxy -R -J -O -K -m -Wthinnest << EOF >> GMT_atan.ps
>
-5	0
5	0
>
0	-1
0	1
EOF
grdgradient -A45 ../../share/doc/gmt/tutorial/us.nc -Nt -M -G$$.tt.nc
grd2xyz -Z $$.tt.nc > $$.d
pshistogram $$.d -R-0.75/0.75/0/5 -JX1.5/1 -B0.5/2f1WSne -W0.01 -O  -K -Gblack -X1.85 -Z1 >> GMT_atan.ps
pstext -R -J -O << EOF >> GMT_atan.ps
-0.7	4.3	9	0	0	LB	tan@+-1@+ 
-0.7	3.7	9	0	0	LB	transformed
EOF
rm -f $$.*
