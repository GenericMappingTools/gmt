#!/usr/bin/env bash
#
# Show error bars on top of a bar plot
ps=error_bars.ps

printf("1 20 2\n2 35 3\n3 30 4\n4 35 1\n5 27 2\n") | gmt psxy lixo.dat -R0.5/5.5/0/40 -JX12c/8c -Ey -Gred -Sb0.35u+b0 -Ba -P > $ps
