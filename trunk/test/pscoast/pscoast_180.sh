#!/bin/bash
#	$Id$
#

ps=pscoast_180.ps

pscoast -R-10/180/20/50 -JM6i -Ba20g20/a20g20+10 -Dc -G221/204/170 -A0/0/1 -P -Xc -K > $ps
pscoast -R0/180/20/50 -JM6i -Ba20g20/a20g20+10 -Dc -G221/204/170 -A0/0/1 -O -Y2.5i >> $ps
