#!/usr/bin/env bash
#
# Plot psxy quoted line symbols for use on man page

ps=GMT_base_symbols8.ps

gmt math -T0/360/1 T 5 MUL COSD 8 MUL = t.txt
# Just a cosine with line-following constant text a few times
gmt psxy -R0/360/-10/10 -JM6i -P -W2.5p,6_4:0 -Sqn4:+l"wiggly line"+v+f12p,Times-Roman,red t.txt --PS_LINE_CAP=round > $ps
