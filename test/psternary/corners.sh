#!/usr/bin/env bash
# Calibration plot for ternary with or without axes reversal
# Ensure psternary places points correctly by plotting the
# three cardinal corners and a 4th point
# Pick (a,b,c) for the three corners (100%) and a 4th point
ps=corners.ps
cat << EOF > t.txt
1	0	0	0
0	1	0	1
0	0	1	2
0.3	0.2	0.5	3
EOF
gmt makecpt -Cred,green,blue,yellow -T0/4/1 > t.cpt
# Plot points using default counter-clockwise axes directions
gmt psternary t.txt -R0/100/0/100/0/100 -JX11c -P -Xc -Baafg+l"A"+u" %" -Bbafg+l"B"+u" %" \
-Bcagf+l"C"+u" %" -La/b/c -B+gbisque -Sc0.5c -Wthin -Ct.cpt -N -K > $ps
gmt pstext -R0/1/0/0.9 -JX6.5c/3c -O -K -F+f12p+jBL -X-3c -Y8c -B0 << EOF >> $ps
0.05	0.7	a = 1, b = c = 0 [red]
0.05	0.5	b = 1, a = c = 0 [green]
0.05	0.3	c = 1, a = b = 0 [blue]
0.05	0.1	a = 0.3 b = 0.2 c = 0.5 [yellow]
EOF
# Now plot again with clockwise axes directions
gmt psternary t.txt -R0/100/0/100/0/100 -JX-11c -O -Baafg+l"A"+u" %" -Bbafg+l"B"+u" %" \
-Bcagf+l"C"+u" %" -B+gbisque -La/b/c -Sc0.5c -Wthin -Ct.cpt -N -X3c -Y5c >> $ps
