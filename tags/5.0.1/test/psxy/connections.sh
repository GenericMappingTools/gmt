#!/bin/bash
#	$Id$
#
# Plot a lot of line segments

. functions.sh
header "Test psxy with a lot of line segments"

pscoast -R-71.6/-57.4/37.4/46.2 -JM20c -Dh -Glightgreen -Wdarkgreen -K > $ps
echo ; echo -n "- test with >  "
time cat "$src"/connections.dat | psxy -A -R -J -O -K -Wred >> $ps
echo -n "- test with NaN"
time sed 's:>:NaN NaN:' "$src"/connections.dat | psxy -A -R -J -O -K -Wblue >> $ps
psbasemap -R -J -O -Ba2f1g2 >> $ps

printf "%-72s" "- PostScript result"
pscmp
