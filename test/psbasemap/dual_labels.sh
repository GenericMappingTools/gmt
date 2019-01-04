#!/usr/bin/env bash
#
# Demonstrates use of dual (left vs right, bottom vs top) Cartesian axis labels
# specified via -B +l"label" and +s"label" strings.
ps=dual_labels.ps

gmt psbasemap -R0/50/0/7 -JX6i/8i -Xc -P -Bxaf+l"Bottom label"+s"Top label" -Byaf+l"Left label"+s"Right label" > $ps
