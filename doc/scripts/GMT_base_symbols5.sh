#!/usr/bin/env bash
#
# Demonstrate a few arrows
gmt begin GMT_base_symbols5
gmt set GMT_THEME cookbook
# Cartesian straight arrows
	gmt plot -R0/5/0/5 -JX1.75i -S -W2p -Gred --MAP_VECTOR_SHAPE=0.5 << EOF
0.5	1.0	4.5	2.5	v0.3i+s+e+bi
EOF
# Circular arrows
	gmt plot -S -W2p -Gred --MAP_VECTOR_SHAPE=0.5 -X2i << EOF
1	0.3	2c	0	90	m0.3i+bt+e
EOF
# Geo arrows
	gmt plot -R0/90/-41.17/41.17 -JM1.75i -S -W2p -Gred --MAP_VECTOR_SHAPE=0.5 -X2i --MAP_FRAME_TYPE=plain << EOF
10	-35	80	8000	=0.3i+b+er
EOF
gmt end show
