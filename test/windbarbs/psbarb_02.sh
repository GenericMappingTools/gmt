#!/usr/bin/env bash
#
# psbarb_02.sh
#   3-D plot with -JZ -p
#

ps=psbarb_02.ps
title="$ps  3-D plot with -JZ -p"

awk '
function abs(x) { return x>=0 ? x : -x }
BEGIN {
  for(y = -90; y <= 90; y += 10)
    for(x = 0; x <= 360; x += 10) {
      z = y + 90
      dir = x
      spd = abs(y) / 2
      print x,y,z, dir, spd
    }
}' > wind.txt

gmt psbarb wind.txt -Q0.4c -Wdefault,black -JQ15c -R0/360/-90/90/0/200 -Ba -BwESnZ1234+t"$title" -JZ6c -Bza50 -p150/45 -P > $ps

rm -f wind.txt
