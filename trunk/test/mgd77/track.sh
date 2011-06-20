#!/bin/bash
#
#       $Id: track.sh,v 1.2 2011-06-20 20:56:58 guru Exp $

. ../functions.sh
header "Testing mgd77track for mapping"

ps=track.ps

pscoast -R200/204/18.5/25 -JM5i -P -B1f30mWSne:."Leg C2308 or 01010221": -K -Gbrown -Sazure2 -Dh --FORMAT_GEO_MAP=dddF -Xc > $ps
mgd77track 01010221 -R -J -O -W0.5p >> $ps

pscmp
