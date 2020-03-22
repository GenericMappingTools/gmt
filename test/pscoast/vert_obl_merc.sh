#!/usr/bin/env bash

# Test the +v modifier to get the oblique Equator aligned with y-axis
# for both northern and southern hemisphere use

ps=vert_obl_merc.ps

gmt set MAP_ANNOT_OBLIQUE 0 MAP_ANNOT_MIN_SPACING 0.5i
gmt pscoast -Gred -R122W/35N/107W/22N+r -JOa120W/25N/150/12c -Bafg -P -K -X5c -Y1.5c > $ps
gmt pscoast -Gred -R-1000/1000/-500/500+uk -JOa173:17:02E/41:16:15S/35/12c -Bafg -O -K -Y5.5c >> $ps
gmt pscoast -Gred -R122W/35N/107W/22N+r -JOa120W/25N/150/12c+dh+v -Bafg -O -K -Y7c >> $ps
gmt pscoast -Gred -R-1000/1000/-500/500+uk  -JOa173:17:02E/41:16:15S/35/12c+v+dh -Bafg -O -X6c >> $ps
