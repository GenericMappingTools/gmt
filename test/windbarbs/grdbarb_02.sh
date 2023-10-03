#!/usr/bin/env bash
#
# grdbarb_02.sh
#   Lambert Projection -JL
#   Set color palette with -C
#

ps=grdbarb_02.ps
title="Lambert Projection, color with -C"
ropt=`
gmt mapproject -Jl135/30/30/60/1:1 -R135/136/30/31 -Fk -I <<EOF \
| awk '{ lo = $1; la = $2 ; getline ; printf "%f/%f/%f/%f", lo,la,$1,$2 }'
-10000 -5000
 10000  7000
EOF
`

gmt grdmath -Rg -I4 135 30 SAZ 110 SUB = dir.grd
gmt grdmath -Rg -I4 50000  135 30 SDIST DIV 50 MIN = spd.grd
gmt begin grdbarb_02 ps
	gmt makecpt -T0/50/5 -Z
	gmt grdbarb spd.grd dir.grd -Z -C -W -JL135/30/60/30/14c -R${ropt}+r -B -B+t"$title"
	gmt colorbar -C -DJMR
gmt end
rm -f *.grd wind.cpt
