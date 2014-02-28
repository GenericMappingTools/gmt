#!/bin/bash
#
#       $Id$

ps=track.ps

ln -fs "${src:-.}"/01010221.mgd77 .
gmt pscoast -R200/204/18.5/25 -JM5i -P -B1f30m -BWSne+t"Leg C2308 or 01010221" -K -Gbrown -Sazure2 -Dh --FORMAT_GEO_MAP=dddF -Xc > $ps
gmt mgd77track 01010221 -R -J -O -W0.5p >> $ps

