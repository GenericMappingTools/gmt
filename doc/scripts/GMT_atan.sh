#!/usr/bin/env bash
gmt begin GMT_atan
	gmt grdgradient -A45 @tut_relief.nc -N -fg -Gtt.t.nc
	gmt grd2xyz -Z tt.t.nc > tt.d
	gmt histogram tt.d -R-0.75/0.75/0/20 -JX1.5i/1i -Bx0.5 -By5f5 -BWSne -W0.01 -Gblack -Z1

	gmt text -F+f9p+jLB << EOF
-0.7	17	Raw
-0.7	15	slopes
EOF
	gmt math -T-5/5/0.01 T ATAN PI DIV 2 MUL = > tt.d
	gmt plot tt.d -R-5/5/-1/1 -Bx2f1g1 -By1f0.5g0.25 -BWSne -Wthick -X1.85i
	gmt plot -Sv5p+e -Gblack -W0.5p << EOF
3	0.8	180	0.45
3	0.8	-90	0.4
EOF
	gmt plot -Wthinnest << EOF
>
-5	0
5	0
>
0	-1
0	1
EOF
	gmt grdgradient -A45 @tut_relief.nc -Nt -fg -Gtt.tt.nc
	gmt grd2xyz -Z tt.tt.nc > tt.d
	gmt histogram tt.d -R-0.75/0.75/0/5 -Bx0.5 -By2f1 -BWSne -W0.01  -Gblack -X1.85i -Z1
	gmt text -F+f9p+jLB << EOF
-0.7	4.3	tan@+-1@+
-0.7	3.7	transformed
EOF
gmt end show
