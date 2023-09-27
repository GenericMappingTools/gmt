#!/usr/bin/env bash
#
# grdbarb_02.sh
#   Lambert Projection -JL
#   Set color palette with -C
#

ps=grdbarb_02.ps
title="$ps  Lambert Projection, color with -C"
ropt=`
gmt mapproject -Jl135/30/30/60/1:1 -R135/136/30/31 -Fk -I <<EOF \
| awk '{ lo = $1; la = $2 ; getline ; printf "%f/%f/%f/%f", lo,la,$1,$2 }'
-10000 -5000
 10000  7000
EOF
`

gmt grdmath -Rg -I4 135 30 SAZ 110 SUB = dir.grd
gmt grdmath -Rg -I4 50000  135 30 SDIST DIV 50 MIN = spd.grd
gmt makecpt -T0/50/5 -Z > wind.cpt

gmt grdbarb spd.grd dir.grd -Z -C./wind.cpt -W -JL135/30/60/30/14c -R${ropt}+r -Ba30g30 -B+t"$title" -K -P > $ps
gmt psscale -C./wind.cpt -J -R -DJMR -O >> $ps

rm -f *.grd wind.cpt
