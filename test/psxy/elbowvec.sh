#!/usr/bin/env bash
# Test broken/elbow arrows: -Sv combined with -Ax (horizontal-first) or -Ay (vertical-first).
# Three arrows 120 degrees apart (30/150/330 deg Cartesian CCW-from-E).
# Tails at three corners, tips all near centre, so the L-legs are clearly visible
# and non-overlapping in both -Ax and -Ay modes.
#
# With -JX3i/-R-3/3/-3/3: scale = 0.5 in/unit, 1.5i = 3 data units.
#   Arrow 1: tail(-2,-2) angle=30  tip( 0.60,-0.50)  -Ax elbow at( 0.60,-2)
#   Arrow 2: tail( 2, 0) angle=150 tip(-0.60, 1.50)  -Ax elbow at(-0.60, 0)
#   Arrow 3: tail( 0, 2) angle=330 tip( 2.60, 0.50)  -Ax elbow at( 2.60, 2)
ps=elbowvec.ps

cat << EOF > elbowvec.txt
-2  -2   30  1.5i
 2   0  150  1.5i
 0   2  330  1.5i
EOF

R=-R-3/3/-3/3
J=-JX3i

# Left panel: horizontal-first elbow (-Ax)
gmt psxy elbowvec.txt $R $J -P -Bafg1 -BWSne+t"Ax" \
    -Sv0.2i+e -Gred -W1p -Ax -K -X1.25i -Y4.5i > $ps

# Right panel: vertical-first elbow (-Ay)
gmt psxy elbowvec.txt $R $J -O -Bafg1 -BWSne+t"Ay" \
    -Sv0.2i+e -Gred -W1p -Ay -X3.75i >> $ps
