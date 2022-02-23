#!/usr/bin/env bash
# Create cubic data in x and y separately, then fit with -N4 +x|y

gmt begin trend_xy_plot ps
    # Fit in x only
    gmt grdmath -R-1.2/1.2/-0.2/0.2 -I0.4 -r 0.5 X ADD X X MUL ADD X 3 POW ADD = data.grd
    gmt grdtrend data.grd -N4+x -Ttrend.grd
    gmt grd2xyz -o0,2 data.grd  | gmt plot -JX10c -B+t"data and model -N4 for x" -R-1/1/-1/2 -Sc0.1c -Gred -B
    gmt grd2xyz -o0,2 trend.grd | gmt plot -W0.5p
    # Fit in x only
    gmt grdmath -R-0.2/0.2/-1.2/1.2 -I0.4 -r 0.5 Y ADD Y Y MUL ADD Y 3 POW ADD = data.grd
    gmt grdtrend data.grd -N4+y -Ttrend.grd
    gmt grd2xyz -o1,2 data.grd  | gmt plot  -B+t"data and model -N4 for y" -R-1/1/-1/2 -Sc0.1c -Gred -B -Y12c
    gmt grd2xyz -o1,2 trend.grd | gmt plot -W0.5p
gmt end show
