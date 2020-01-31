#!/usr/bin/env bash
#

ps=onelayer.ps

gmt grdinterpolate "${src:-.}"/cube.nc -T4 -Gnewlayer.nc
gmt grdcontour newlayer.nc -A20 -C10 -JX6i -Baf -BWNse+t"Interpolate new layer between z = 3 and 5" -GlBL/TR -P -Xc > $ps
