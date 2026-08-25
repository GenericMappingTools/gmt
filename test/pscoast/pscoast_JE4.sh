#!/usr/bin/env bash
#
# GMT test script for a global -JE Azimuthal equidistant map.

ps=pscoast_JE4.ps
gmt pscoast -Rg -JE-100/40/20c -Bg15 -A10000 -B+glightblue -Dc -Gblack -P -Xc > $ps
