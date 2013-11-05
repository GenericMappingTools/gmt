#!/bin/bash
# Test the kml2gmt application

. functions.sh
header "Test kml2gmt by recovering data produced by gmt2kml"

pscoast -R-11/2/49:50/59:30 -JM6i -M -W0.25p -Di > coast.txt
psxy -R -J -P -K coast.txt -W2p,green -Baf -Xc > $ps
kml2gmt "$src"/coast.kml | psxy -R -J -O -K -W0.25p,red >> $ps
psxy -R -J -O -T >> $ps

pscmp
