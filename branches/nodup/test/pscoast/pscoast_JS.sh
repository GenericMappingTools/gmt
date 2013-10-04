#!/bin/bash
#	$Id$
#

ps=pscoast_JS.ps

gmt pscoast -JS0/90/7i -R-180/180/65/80 -Dc -A1000 -Gred -Sblue -Bx30g30f10 -Byg10 -BSn --MAP_FRAME_TYPE=PLAIN > $ps

