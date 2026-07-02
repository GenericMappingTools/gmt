#!/usr/bin/env bash
#
# Test auto-legend for front (fault) symbols (-Sf).
#
# GRAPHICSMAGICK_RMS = 0.02
# GMT_KNOWN_FAILURE
#
# Regression test for issue #9066:
#  1) Each auto-legend entry must show the correct front sub-type
#     (e.g., +c circle vs +b box), not always a box.
#  2) An entry with no explicit -W must not inherit the pen from a
#     previous entry that did specify one (here, "circle thick" uses
#     an explicit thick red pen, while "box" specifies no pen at all
#     and must fall back to the default pen, not thick red).
cat << EOF > t1.txt
0 1
1 1
EOF
cat << EOF > t2.txt
0 2
1 2
EOF
gmt begin autolegendfront ps
	gmt basemap -R0/1/0/5 -JX5c -Bf
	gmt plot -Wthick,red -Gblack  -Sf2+c+r t2.txt -l"circle thick"
	gmt plot             -Gpurple -Sf2+b+r t1.txt -l"box"
gmt end

rm t*.txt