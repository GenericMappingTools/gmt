#!/usr/bin/env bash
#
# psbarb_01.sh
#   Stereographic Projection -JS
#

ps=psbarb_01.ps
title="Stereographic Projection"

awk '
function abs(x) { return x>=0 ? x : -x }
BEGIN {
  for(y = -90; y <= 90; y += 10)
    for(x = 0; x <= 360; x += 10) {
      dir = x
      spd = abs(y) / 2
      print x,y, spd, dir, spd
    }
}' > wind.txt
gmt makecpt -T0/45/5 -Z > wind.cpt

gmt psbarb wind.txt -Q0.4c -C./wind.cpt -JS-60/90/14c -R0/360/-20/90 -Ba30g30 -B+t"$title" -K -P > $ps
gmt psscale -C./wind.cpt -J -R -DJMR -O >> $ps

rm -f wind.txt wind.cpt
