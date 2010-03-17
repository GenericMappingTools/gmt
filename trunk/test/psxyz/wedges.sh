#!/bin/sh
#	$Id: wedges.sh,v 1.1 2010-03-17 20:51:05 guru Exp $
#
# Plot wedges with directions and azimuths

. ../functions.sh
header "Test psxyz wedges for directions and azimuths"

ps=wedges.ps
echo 0 0 0 0 2 | psxyz -SVB0.05/0/0 -R-1/1/-1/1 -Ba.5g1WSne -JX2.5i -P -K -Gred -Y6i > $ps
echo 0 0 0 2 -20 20 | psxyz -SW -R -J -O -K -W1,blue >> $ps

echo 0 0 0 0 2 | psxyz -SVB0.05/0/0 -R-1/1/-1/1 -Ba.5g1WSne -JM2.5i -O -K -Gred -X3.5i >> $ps
echo 0 0 0 2 -20 20 | psxyz -SW -R -J -O -K -W1,blue >> $ps

echo 0 0 0 0 2 | psxyz -SvB0.05/0/0 -R-1/1/-1/1 -Ba.5g1WSne -JX -E135/35 -O -K -Gred -X-3.5i -Y-5i >> $ps
echo 0 0 0 2 0 30 | psxyz -Sw -R -J -O -K -E135/35 -W1,blue >> $ps

echo 0 0 0 0 2 | psxyz -SvB0.05/0/0 -R-1/1/-1/1 -Ba.5g1WSne -JM -E135/35 -O -K -Gred -X3.5i -Y1i >> $ps
echo 0 0 0 2 0 30 | psxyz -Sw -R -J -O -E135/35 -W1,blue >> $ps

pscmp
