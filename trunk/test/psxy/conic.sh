#!/bin/bash
#	$Id: conic.sh,v 1.2 2011-03-15 02:06:46 guru Exp $
#
# Check clipping of line for a global conic plot

. ../functions.sh
header "Test psxy for clipping lines with periodic conic boundary"

ps=conic.ps
pscoast -R0/360/30/70 -JL180/50/40/60/6i -Gred -Dc -B30g30 -P -K > $ps
psxy -R -J -O -W2p << EOF >> $ps
30	50
-30	45
EOF

pscmp
