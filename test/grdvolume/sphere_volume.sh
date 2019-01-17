#!/usr/bin/env bash
#
# Make a sphere of radius 10 using two half grids and compute volumnes
# Addressing issue #895
ps=sphere_volume.ps
gmt grdmath -R-10.1/10.1/-10.1/10.1 -I0.1 X Y HYPOT 10 DIV ACOS SIN 10 MUL NEG = bot_half.grd
gmt grdmath bot_half.grd NEG = top_half.grd
T=(`gmt grdvolume top_half.grd -C0 --FORMAT_FLOAT_OUT=%.3f`)
B=(`gmt grdvolume bot_half.grd -Cr-10/0 --FORMAT_FLOAT_OUT=%.3f`)
gmt grdgradient top_half.grd -Nt1 -A45 -Gint.grd
echo "-100 gray 100 gray" > t.cpt
gmt grdview bot_half.grd -Iint.grd -Ct.cpt -Qi100 -Jx0.2i -Jz0.2i -p125/35 -P -Baf -BwSnE -K -X1.5i > $ps
echo "Area = ${B[1]} Volume = ${B[2]}" | gmt pstext -R -J -Jz -O -K -p -F+f18p+cBC+jTC -Dj0/0.4i -N >> $ps
gmt grdview top_half.grd -Iint.grd -Ct.cpt -Qi100 -Jx0.2i -Jz0.2i -p125/35 -P -Baf -BwSnE -O -K -Y5.25i >> $ps
echo "Area = ${T[1]} Volume = ${T[2]}" | gmt pstext -R -J -Jz -O -K -p -F+f18p+cBC+jTC -Dj0/0.4i -N >> $ps
gmt psxy -R -J -O -T >> $ps
