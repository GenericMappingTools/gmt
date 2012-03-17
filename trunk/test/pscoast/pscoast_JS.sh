#!/bin/bash
#	$Id$
#

ps=pscoast_JS.ps

pscoast -JS0/90/7i -R-180/180/65/80 -Dc -A1000 -Gred -Sblue -B30g30f10/g10Sn --MAP_FRAME_TYPE=PLAIN > $ps

