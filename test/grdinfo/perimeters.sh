#!/usr/bin/env bash
# Test grdinfo -Ib for both pixel and gridline-registered grids.

cat << EOF > p_answer.txt
0	0
0.5	0
1.5	0
2	0
2	0.5
2	1.5
2	2
1.5	2
0.5	2
0	2
0	1.5
0	0.5
0	0
EOF
cat << EOF > g_answer.txt
0	0
1	0
2	0
2	1
2	2
1	2
0	2
0	1
0	0
EOF
gmt grdmath -R0/2/0/2 -I1 -rg 0 = g.grd
gmt grdmath -R0/2/0/2 -I1 -rp 0 = p.grd
gmt grdinfo -Ib g.grd > g_result.txt
gmt grdinfo -Ib p.grd > p_result.txt

diff g_result.txt g_answer.txt --strip-trailing-cr > fail
diff p_result.txt p_answer.txt --strip-trailing-cr >> fail
