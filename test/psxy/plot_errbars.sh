#!/usr/bin/env bash
#
# Plot error bars and test [c+l|f]+p<epen>

ps=plot_errbars.ps

cat << EOF > tt.d
1	1	1	1
2	2	2	1
3	3	3	1
4	4	4	1
5	5	5	1
EOF
gmt makecpt -Crainbow -T0/6/1 > tt.cpt
gmt psxy -R0/6/0/6 -JX3i -P -B0 -Sc0.2i -Ctt.cpt -W0.25p -X1i -Y2i tt.d -Ex+p2p,red -K > $ps
gmt psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W0.25p -X3.25i tt.d -Ey+cl+p1p -K >> $ps
gmt psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W5p+c -X-3.25i -Y3.5i tt.d -Ey+cf+p1p -K >> $ps
gmt psxy -R -J -O -B0 -Sc0.2i -Ctt.cpt -W0.25p,red -X3.25i tt.d -Ex+cl+p1p >> $ps
