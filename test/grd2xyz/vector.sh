#!/usr/bin/env bash
# Testing gmt grd2xyz with -L
# The answer.txt file has been vetted by eye

cat << EOF > answer.txt
20	50	1000
20	40	800
20	30	600
20	20	400
20	10	200
20	0	0
20	50	1000
20	40	800
20	30	600
20	20	400
20	10	200
20	0	0
0	30	0
10	30	300
20	30	600
30	30	900
40	30	1200
50	30	1500
0	30	0
10	30	300
20	30	600
30	30	900
40	30	1200
50	30	1500
EOF
gmt grdmath -R0/50/0/50 -I10 X Y MUL = tmp.grd
gmt grd2xyz tmp.grd -Lc2 > result.txt
gmt grd2xyz tmp.grd -Lx20 >> result.txt
gmt grd2xyz tmp.grd -Lr2 >> result.txt
gmt grd2xyz tmp.grd -Ly30 >> result.txt
diff answer.txt result.txt --strip-trailing-cr > fail
