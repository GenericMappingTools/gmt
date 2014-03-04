#!/bin/bash
#
#	$Id: oblique.sh 9545 2011-07-27 19:31:54Z pwessel $

ps=oblique.ps

gmt gmtset MAP_ANNOT_OBLIQUE 14 MAP_ANNOT_MIN_SPACING 0.5i
gmt psbasemap -R-100/100/-60/60 -JOa1/0/45/5.5i -B30g30 -P -K -Xc > $ps
gmt psbasemap -R -JOa0/0.1/45/5.5i -B30g30 -O -Y5i >> $ps
