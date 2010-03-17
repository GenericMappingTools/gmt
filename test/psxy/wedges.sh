#!/bin/sh
#	$Id: wedges.sh,v 1.1 2010-03-17 20:51:05 guru Exp $
#
# Plot wedges with directions and azimuths

. ../functions.sh
header "Test psxy wedges for directions and azimuths"

ps=wedges.ps
echo 0 0 0 2 | psxy -SVB0.05/0/0 -R-1/1/-1/1 -B1g1WSne -JX3i -K -Gred -P -Y6i > $ps
echo 0 0 2 -20 20 | psxy -SW -R -J -O -K -W1p,blue >> $ps

echo 0 0 0 2 | psxy -SVB0.05/0/0 -R-1/1/-1/1 -B1g1WSne -JM3i -O -K -Gred -X3.75i >> $ps
echo 0 0 2 -20 20 | psxy -SW -R -J -O -K -W1p,blue >> $ps

echo 0 0 0 2 | psxy -SvB0.05/0/0 -R-1/1/-1/1 -B1g1WSne -JX3i -O -K -Gred -X-3.75i -Y-4i >> $ps
echo 0 0 2 0 30 | psxy -Sw -R -J -O -K -W1p,blue >> $ps

echo 0 0 0 2 | psxy -SvB0.05/0/0 -R-1/1/-1/1 -B1g1WSne -JM3i -O -K -Gred -X3.75i >> $ps
echo 0 0 2 0 30 | psxy -Sw -R -J -O -W1p,blue >> $ps

pscmp
