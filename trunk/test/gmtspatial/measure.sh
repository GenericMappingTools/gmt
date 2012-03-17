#!/bin/bash
# $Id$
#

cat << EOF > area.txt
0	0
1	0
1	1
0	1
0	0
EOF
rm -f fail
# Cartesian centroid and area
echo "0.5	0.5	1" > answer
gmtspatial -Q area.txt > result
diff -q --strip-trailing-cr answer result >> fail
# Geographic centroid and area
echo "0.5	0.500019038226	12308.3096995" > answer
gmtspatial -Q area.txt  -fg > result
diff -q --strip-trailing-cr answer result >> fail
