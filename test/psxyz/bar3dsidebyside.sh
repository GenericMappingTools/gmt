#!/usr/bin/env bash
# Test flat 2-D multiband side-by-side bars in 3-D view
cat << EOF > testy.txt
1	2	0	5	3	8
2	1	0	6	1	5
EOF
cat << EOF > testx.txt
2	1	0	5	3	8
1	2	0	6	1	5
EOF
gmt begin bar3dsidebyside ps
	gmt makecpt -Cjet -T0/4/1
	gmt subplot begin 2x1 -Fs10c
		gmt plot3d testy.txt -R0/3/0/25 -C -W1p -Sb2c+i4+s20+B2 -p135/35 -c
		printf "0 2 0\n3 2 0\n" | gmt plot3d -W0.25p,dashed -p
		gmt plot3d testx.txt -R0/25/0/3 -C -W1p -SB2c+B2+i4+s40 -p135/35 -c
		printf "2 0 0\n2 3 0\n" | gmt plot3d -W0.25p,dashed -p
	gmt subplot end
gmt end show
