#!/usr/bin/env bash
#

ps=stereonet_density.ps

# Regression test for -D's fabs() fold (see psstereonet_density_contour): 090/03, 090/02,
# 090/04 and 270/03, 270/02, 270/04 are the SAME three E-W lines, each recorded from the
# opposite end.  With fabs() in the density kernel they must contour as one cluster
# straddling the net's rim (lon = +90/-90); dropping fabs() would wrongly split them into
# two separate, weaker maxima and make the field discontinuous across the rim.
cat << EOF > rim_test.txt
88 3
90 2
92 4
268 3
270 2
272 4
EOF

gmt psstereonet rim_test.txt -JA10c -B -Tl -De+i1+s1 -Sc0.1c -Gblack -P > $ps
