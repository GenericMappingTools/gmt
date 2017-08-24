#!/bin/bash
#
#	$Id$
# Demonstrates use of dual (left vs right, bottom vs top) Cartesian axis labels
# specified via -B +l"label" strings.
ps=dual_labels.ps

gmt psbasemap -R0/50/0/7 -JX6i/8i -Xc -P -Bxaf+l"Bottom label||Top label" -Byaf+l"Left label||Right label" > $ps
