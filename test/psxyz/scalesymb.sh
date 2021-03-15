#!/usr/bin/env bash
#
# Test -H for overall symbol and pen scaling via data column
# The data column follows after optional z and size columns.
cat << EOF > a.txt
# -S sets no size, must read variable size rom file
# x y	z   w   size scale
> -W2p
0	0	0	3	1i		0.9
1	1	0	5	1.8i	0.3
> -W2p,blue,-
-1	-1	0	4	0.8i	0.5
1	-1	0	6	0.8i	1.3
EOF
cat << EOF > b.txt
# A constant size is set via -S instead
# x y	z	w  scale
> -W2p
0	0	0	3	0.9
1	1	0	5	0.3
> -W2p,blue,-
-1	-1	0	4	0.5
1	-1	0	6	1.3
EOF
gmt begin scalesymb ps
	gmt makecpt -Cjet -T2/6
	gmt subplot begin 2x1 -R-2/2/-2/2 -Fs10c -SCl -SRb
		gmt plot3d -H -Sc -C a.txt   -c
		gmt plot3d -H -Sc1.2i -C b.txt -c
	gmt subplot end
gmt end show
