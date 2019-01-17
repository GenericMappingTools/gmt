#!/usr/bin/env bash
#
# Check clipping of line for a global conic plot

ps=conic.ps

gmt pscoast -R0/360/30/70 -JL180/50/40/60/6i -Gred -Dc -B30g30 -P -K > $ps
gmt psxy -R -J -O -W2p << EOF >> $ps
30	50
-30	45
EOF
