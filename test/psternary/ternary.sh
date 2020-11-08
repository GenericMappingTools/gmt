#!/usr/bin/env bash
# Test ternary diagram plotting a dataset from Matlab exchange
ps=ternary.ps
gmt makecpt -T0/70 -Cjet > t.cpt
gmt psternary ${src:-.}/ternary.txt -R0/100/0/100/0/100 -JX-6i -P -Xc -Baafg+l"Water component"+u" %" -Bbafg+l"Air component"+u" %" \
-Bcagf+l"Limestone component"+u" %" -B+givory+t"Example data from MATLAB Central" -Sc0.1c -Ct.cpt -K -Y2i -LWater/Air/Limestone > $ps
gmt psscale -R0/100/0/100 -J -O -Ct.cpt -DJCB+o0/0.75i -Baf >> $ps
