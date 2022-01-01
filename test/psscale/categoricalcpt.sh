#!/usr/bin/env bash
# See issue # 3766.  Fixed in #4407
# Only actual categorical values will get a color and bad values are given NaN color
# Also, the back/foreground triangles are not accessible for categorical CPTs

gmt begin categoricalcpt ps

cat > types.cpt << EOF
0	yellow	;desert
1	blue	;forest
2	red		;iceland
B	white
F	black
N	gray
EOF
cat << EOF > tmp.txt
1	3	-1
2	3	0
3	3	1
4	3	1.5
5	3	2
6	3	2.01
7	3	3
8	3	3.1
9	3	NaN
EOF
gmt plot -R0/10/0/6 -Jx1.5c -Baf -BWSen -Sc1c -W1p -Ctypes.cpt tmp.txt
gmt text -F+f12p -D0/1c tmp.txt

gmt colorbar -Ctypes.cpt -DJBC+e+n
gmt end show
