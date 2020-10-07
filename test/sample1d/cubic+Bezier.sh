#!/usr/bin/env bash
# Compare cubic spline and Bezier line
ps=cubic+Bezier.ps
cat << EOF > t.txt
0       0
1       1
2       1.5
3       1.25
4       1.5
4.5     3
5       2
6       2.5
EOF
# Plot using Bezier scheme
gmt psxy -R-0.1/6.1/-0.1/3.1 -JX6i/3i -P t.txt -W1p+s -Baf -BWSne -K -Y0.75i -Xc > $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gblue -Wthin >> $ps
# Plot using regular sample1d scheme
gmt sample1d t.txt -S0/6 -I0.01 -Fc | gmt psxy -R-0.1/6.1/-0.1/3.1 -J -O -Bafg -BWsne -W1p -K -Y3.2i >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gblue -Wthin >> $ps
# Compare cubic sspline and Bezier
gmt sample1d t.txt -S0/6 -I0.01 -Fc | gmt psxy -R-0.1/6.1/-0.1/3.1 -J -O -Bafg -BWsne -Wfaint,black -K -Y3.2i >> $ps
gmt psxy -R -J -O -K t.txt -Wfaint,red+s >> $ps
gmt psxy -R -J -O t.txt -Sc0.1i -Gblue -Wthin >> $ps
