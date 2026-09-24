#!/usr/bin/env bash
# Test physically based shading (-F): cast shadows, ambient occlusion and tone mapping on the
# left, a rough metal with a vertical exaggeration of 3 on the right, and the -I+f defaults below

ps=pbr.ps

gmt grdmath -R-15/15/-15/15 -I0.1 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
gmt makecpt -Cjet -T-0.5/1 > pal.cpt

gmt grdimage somb.nc -Cpal.cpt -F225/30+o+s+t -JX8c -Baf -BWSne -P -K -Y14c > $ps
gmt grdimage somb.nc -Cpal.cpt -F225/30+m0.8+r0.6+v3 -J -Baf -BwSne -O -K -X9c >> $ps
gmt grdimage somb.nc -Cpal.cpt -I+f -J -Baf -BWSne -O -X-9c -Y-10c >> $ps

rm -f somb.nc pal.cpt
