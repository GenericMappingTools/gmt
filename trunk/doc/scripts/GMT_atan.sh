#!/bin/bash
#	$Id$
#
. ./functions.sh

grdgradient -A45 "$src"/../tutorial/us.nc -N -fg -Gtt.t.nc
grd2xyz -Z tt.t.nc > tt.d
pshistogram tt.d -R-0.75/0.75/0/20 -JX1.5/1 -B0.5/5f5WSne -W0.01 -P -K -Gblack -Z1 > GMT_atan.ps

pstext -R -J -O -K -F+f9p+jLB << EOF >> GMT_atan.ps
-0.7	17	Raw 
-0.7	15	slopes
EOF
gmtmath -T-5/5/0.01 T ATAN PI DIV 2 MUL = > tt.d
psxy tt.d -R-5/5/-1/1 -JX1.5/1 -B2f1g1/1f0.5g0.25WSne -O -K -Wthick -X1.85 >> GMT_atan.ps
psxy -R -J -O -K -Sv5p+e -Gblack -W0.5p << EOF >> GMT_atan.ps
3	0.8	180	0.45
3	0.8	-90	0.4
EOF
psxy -R -J -O -K -Wthinnest << EOF >> GMT_atan.ps
>
-5	0
5	0
>
0	-1
0	1
EOF
grdgradient -A45 "$src"/../tutorial/us.nc -Nt -fg -Gtt.tt.nc
grd2xyz -Z tt.tt.nc > tt.d
pshistogram tt.d -R-0.75/0.75/0/5 -JX1.5/1 -B0.5/2f1WSne -W0.01 -O  -K -Gblack -X1.85 -Z1 >> GMT_atan.ps
pstext -R -J -O -F+f9p+jLB << EOF >> GMT_atan.ps
-0.7	4.3	tan@+-1@+ 
-0.7	3.7	transformed
EOF
