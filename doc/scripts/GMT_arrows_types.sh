#!/usr/bin/env bash
#
# Demonstrate how arrows heads can look like (just doing Cartesian arrows here)
gmt begin GMT_arrows_types
# Cartesian straight arrows
	gmt plot -R0/10/0/7 -JX5.75i/1.75i -S -W1.5p -Gred -B0 --MAP_VECTOR_SHAPE=0 << EOF
0.5	0.5	4.5	0.5	v0.25i+s+e+a40
0.5	1.5	4.5	1.5	v0.25i+s+b
0.5	2.5	4.5	2.5	v0.25i+s+e+b
EOF
	gmt plot -S -W1.5p -Gred --MAP_VECTOR_SHAPE=0.5 << EOF
0.5	3.5	4.5	3.5	v0.25i+s+e+b
0.5	4.5	4.5	4.5	v0.25i+s+b+a45
0.5	5.5	4.5	5.5	v0.25i+s+e+bi
0.9	6.5	4.1	6.5	v0.25i+s+eI+bI
EOF
	gmt plot -S -W1.5p -Gred -X2.875i << EOF
0.5	0.5	4.5	0.5	v0.25i+s+et
0.5	1.5	4.5	1.5	v0.25i+s+bc
0.5	2.5	4.5	2.5	v0.25i+s+et+bt
EOF
	gmt plot -S -W1.5p -Gred --MAP_VECTOR_SHAPE=0.5 << EOF
0.5	3.5	4.5	3.5	v0.25i+s+e+bt
0.5	4.5	4.5	4.5	v0.25i+s+b+ec+a45
EOF
	gmt plot -S -W1.5p -Gred --MAP_VECTOR_SHAPE=0.5 << EOF
0.5	5.5	4.5	5.5	v0.25i+s+b+ei
0.5	6.5	4.5	6.5	v0.25i+s+bA+eA
EOF
gmt end show
