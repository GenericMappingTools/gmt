#!/bin/bash
#       $Id$
#
. ./functions.sh

# Demonstrate how arrows look like
ps=GMT_arrows.ps
# Cartesian straight arrows
psxy -R0/5/0/5 -JX1.75i -P -S -W1.5p -Gred -K -B0 --MAP_VECTOR_SHAPE=0.5 << EOF > $ps
0.5	0.5	4.5	0.5	v0.2i+s
0.5	1.5	4.5	1.5	v0.2i+s+e
0.5	2.5	4.5	2.5	v0.2i+s+b
0.5	3.5	4.5	3.5	v0.2i+s+b+e
0.5	4.5	4.5	4.5	v0.2i+s+l+e
EOF
# Circular arrows
psxy -R -J -S -W1.5p -Gred -O -K -B0 --MAP_VECTOR_SHAPE=0.5 -X2i << EOF >> $ps
0.5	0.5	1.5	0	90	m0.2i
0.5	0.5	1.3	0	90	m0.2i+e
0.5	0.5	1.1	0	90	m0.2i+b
0.5	0.5	0.9	0	90	m0.2i+b+e
0.5	0.5	0.7	0	90	m0.2i+r+e
0.5	0.5	0.3	0	90	M0.2i+r+e
EOF
# Geo arrows
psxy -R0/90/-41.17/41.17 -JM1.75i -S -W1.5p -Gred -O -K -B0 --MAP_VECTOR_SHAPE=0.5 -X2i --MAP_FRAME_TYPE=plain << EOF >> $ps
10	0	90	8000	=0.2i
10	13	90	8000	=0.2i+e
10	-13	90	8000	=0.2i+b
10	25	90	8000	=0.2i+b+e
10	-25	90	8000	=0.2i+b+e
10	35	90	8000	=0.2i+r+b
10	-35	90	8000	=0.2i+l+b
EOF

psxy -R -J -O -T >> $ps
