#!/bin/bash
#		GMT EXAMPLE 07
#		$Id$
#
# Purpose:	Make a basemap with earthquakes and isochrons etc
# GMT progs:	pscoast, pstext, psxy
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_07.ps
pscoast -R-50/0/-10/20 -JM9i -K -Slightblue -GP300/26:FtanBdarkbrown -Dl -Wthinnest \
	-B10 -U"Example 7 in Cookbook" --FORMAT_GEO_MAP=dddF > $ps
psxy -R -J -O -K fz.xy -Wthinner,- >> $ps
psxy quakes.xym -R -J -O -K -h1 -Sci+s0.01 -Gred -Wthinnest >> $ps
psxy -R -J -O -K isochron.xy -Wthin,blue >> $ps
psxy -R -J -O -K ridge.xy -Wthicker,orange >> $ps
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
echo "-13.5 16.5" | psxy -R -J -O -K -Sc0.08i -Gred -Wthinner >> $ps
echo "-12.5 16.5 ISC Earthquakes" | pstext -R -J -F+f18p,Times-Italic+jLM -O -K >> $ps
pstext -R -J -O -F+f30,Helvetica-Bold,white=thin >> $ps << END
-43 -5 SOUTH
-43 -8 AMERICA
 -7 11 AFRICA
END
