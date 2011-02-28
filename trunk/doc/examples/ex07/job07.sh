#!/bin/bash
#		GMT EXAMPLE 07
#		$Id: job07.sh,v 1.10 2011-02-28 00:58:03 remko Exp $
#
# Purpose:	Make a basemap with earthquakes and isochrons etc
# GMT progs:	pscoast, pstext, psxy
# Unix progs:	$AWK, echo, rm
#
. ../functions.sh
ps=../example_07.ps
pscoast -R-50/0/-10/20 -JM9i -K -GP300/26 -Dl -Wthinnest -B10 -U"Example 7 in Cookbook" > $ps
psxy -R -J -O -K -m fz.xy -Wthinner,- >> $ps
$AWK '{print $1-360.0, $2, $3*0.01}' quakes.xym | psxy -R -J -O -K -H1 -Sci -Gwhite -Wthinnest >> $ps
psxy -R -J -O -K -m isochron.xy -Wthin >> $ps
psxy -R -J -O -K -m ridge.xy -Wthicker >> $ps
psxy -R -J -O -K -Gwhite -Wthick -A >> $ps << END
-14.5	15.2
 -2	15.2
 -2	17.8
-14.5	17.8
END
psxy -R -J -O -K -Gwhite -Wthinner -A >> $ps << END
-14.35	15.35
 -2.15	15.35
 -2.15	17.65
-14.35	17.65
END
echo "-13.5 16.5" | psxy -R -J -O -K -Sc0.08i -Gwhite -Wthinner >> $ps
echo "-12.5 16.5 18 0 6 LM ISC Earthquakes" | pstext -R -J -O -K >> $ps
pstext -R -J -O -Sthin -Gwhite >> $ps << END
-43 -5 30 0 1 CM SOUTH
-43 -8 30 0 1 CM AMERICA
 -7 11 30 0 1 CM AFRICA
END
rm -f .gmt*
