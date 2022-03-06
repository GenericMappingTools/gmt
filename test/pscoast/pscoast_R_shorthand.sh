#!/usr/bin/env bash
#
# Introduce the two -R shorthands for projected regions centered on (0,0)

ps=pscoast_R_shorthand.ps

gmt pscoast -R300+un -JA204/21/14c -Glightgray -Baf -K -P -Xc -Y2c > $ps
echo "-R300+un -JA204/21/14c" | gmt pstext -R -J -O -K -F+cBL+f13p -Dj0.1c >> $ps
gmt pscoast -R500/300+uk -JS11/60/14c -Glightgray -Baf -O -K -Y16c >> $ps
echo "-R500/300+uk -JS11/60/14c" | gmt pstext -R -J -O -F+cBL+f13p -Dj0.1c >> $ps
