#!/usr/bin/env bash
# Illustrate Cartesian angle and geographic azimuth via custom symbols
gmt begin GMT_angle-azim
    gmt subplot begin 1x2 -R-1/3/-1/3 -Fs6c -A+jTL -B -BWSrt
    echo 0 0 36 | gmt plot -JM? -Sk@azimuth/4c -W3p -Gred -c --MAP_FRAME_TYPE=plain
    echo 0 0 36 | gmt plot -Jx? -Sk@angle/4c -W3p -Gred  -c
    gmt subplot end
gmt end show
