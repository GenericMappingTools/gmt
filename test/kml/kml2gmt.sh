#!/usr/bin/env bash
# Test the kml2gmt application

ps=kml2gmt.ps

gmt pscoast -R-11/2/49:50/59:30 -JM6i -M -W0.25p -Di > coast.txt
gmt psxy -R -J -P -K coast.txt -W2p,green -Baf -Xc > $ps
gmt kml2gmt "${src:-.}"/coast.kml | gmt psxy -R -J -O -W0.25p,red >> $ps
