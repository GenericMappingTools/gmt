#!/usr/bin/env bash
# Combination of auto-I with oblique -R
ps=autointense.ps
gmt grdimage -I+d @earth_relief_05m -P -JS21/90/15c -R10/68/50/80+r -Bafg -Xc > $ps
