#!/bin/bash
#	$Id: connections.sh,v 1.1 2011-07-14 20:10:39 remko Exp $
#
# Plot a lot of line segments

. ../functions.sh
header "Test psxy with a lot of line segments"

ps=connections.ps

pscoast -R-71.6/-57.4/37.4/46.2 -JM20c -Dh -Glightgreen -Wdarkgreen -K > $ps
echo ; echo -n "- test with >  "
cat connections.dat | time psxy -A -R -J -O -K -Wred >> $ps 
echo -n "- test with NaN"
sed 's:>:NaN NaN:' connections.dat | time psxy -A -R -J -O -K -Wblue >> $ps 
psbasemap -R -J -O -Ba2f1g2 >> $ps

printf "%-72s" "- PostScript result"
pscmp
