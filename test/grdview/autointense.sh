#!/usr/bin/env bash
# Combination of auto-I with oblique -R
ps=autointense.ps
gmt grdview -I @earth_relief_05m -P -JS21/90/12c -R10/68/50/80r -Bafg -p165/35 -Qi100 -JZ1i -Crainbow -X2c > $ps
