#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# Test plotting of Antarctica polygon in different projections.

ps=antwrap.ps

gmt psxy -R110/-79/-55/0r -JN6i -Gred -Bafg GSHHS_l_Antarctica.txt -P -K > $ps
gmt psxy -R0/360/-90/-55 -JA0/-90/5i -O -Gred GSHHS_l_Antarctica.txt -Bafg -X0.5i -Y4.25i >> $ps
