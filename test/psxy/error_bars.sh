#!/usr/bin/env bash
#
# Show error bars on top of a bar plot
ps=error_bars.ps

printf "1 20 2\n2 35 3\n3 30 4\n4 35 1\n5 27 2\n" | gmt psxy -R0.5/5.5/0/40 -JX16c/10c -Ey+p1p -Gred -Sb0.35q+b0 -Ba -P > $ps
