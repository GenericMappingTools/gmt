#!/usr/bin/env bash
# Test 2-D multiband bars with common base
cat << EOF > testy.txt
1	2	5	3	8
2	1	6	1	5
EOF
cat << EOF > testx.txt
2	1	5	3	8
1	2	6	1	5
EOF
gmt begin bar2dmultimode ps
	gmt makecpt -Cjet -T0/4/1
	gmt subplot begin 2x1 -Fs10c
		gmt plot testy.txt -R0/3/0/25 -C -W1p -Sb0.5c+i4+B2 -c
		printf "0 2\n3 2\n" | gmt plot -W0.25p,dashed
		gmt plot testx.txt -R0/25/0/3 -C -W1p -SB0.5c+B2+i4 -c
		printf "2 0\n2 3\n" | gmt plot -W0.25p,dashed
	gmt subplot end
gmt end show
