#!/usr/bin/env bash
# Test that polygons with holes plot correctly regardless of handedness
# for both geo and Cartesian plots
cat << EOF > quads.dat
> -Gred CCW
0  0
5  0
5  10
0  10
0  0
> -Ph  CW
2  1
2  3
4  3
4  1
2  1
> -Ph  CCW
2  4
4  4
4  6
2  6
2  4
> -Ph CCW origin TopLeft
2  9
2  7
4  7
4  9
2  9
> CW -Gblue
5  0
5  10
10 10
10 0
5  0
> -Ph  CW
7  1
7  3
9  3
9  1
7  1
> -Ph  CCW
7  4
9  4
9  6
7  6
7  4
EOF
gmt begin polyholes ps
	gmt plot quads.dat -JX10c -Ba2f1 -BWSen -R-0.5/10.5/-0.5/10.5 -Gred -W1p
	gmt plot quads.dat -JM10c -Ba2f1 -BWSen -Gred -W1p -Y12c
gmt end show
