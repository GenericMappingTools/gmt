#!/usr/bin/env bash
# Test single mask points in psmask, including the poles
ps=holes.ps
gmt math -T-90/90/10 -N2/1 0 = | gmt psmask -Gyellow -I30m -R-75/75/-90/90 -JQ0/7i \
	-S4d -T -Bxafg180 -Byafg10 -BWSne+t"Mask for points with r = 4 degrees" -P -X0.75i > $ps
