#!/usr/bin/env bash
# Test physically based shading (-F) of a 3-D surface: cast shadows, ambient occlusion and tone
# mapping with a vertical exaggeration of 3, draped over the relief it was computed from

ps=pbr.ps

gmt grdmath -R-15/15/-15/15 -I0.1 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = somb.nc
gmt makecpt -Cjet -T-0.5/1 > pal.cpt

gmt grdview somb.nc -Cpal.cpt -F225/30+o+s+t+v3 -JX12c -JZ3c -p135/35 -Baf -Bzaf -BWSneZ -P -Xc > $ps

rm -f somb.nc pal.cpt
