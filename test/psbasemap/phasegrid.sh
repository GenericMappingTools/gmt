#!/bin/bash
#
# This is based on issue #816 where we lost some of the meridians when the phase was nonzero

ps=phasegrid.ps

gmt psbasemap -Rg -JF7.5/45/70/4.5i -Bg15 -P -K -Xc > $ps
gmt psbasemap -R -J -O -Bxg15+7.5 -Byg15 -Y5i >> $ps
