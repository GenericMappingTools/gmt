#!/bin/bash
#	$Id$
#
# Check clipping of lines crossing over the horizon AND dateline (N pole)

ps=clipping1.ps

psxy clipline1.xy -P -K -JG0/90/4.5i -R-180/180/40/90 -B30g10 -Wthick,red -X1i -Y0.75i --MAP_FRAME_TYPE=plain > $ps 
psxy clipline1.xy -O -JG0/90/4.5i -R-180/180/0/90 -B30g10 -Wthick,red -X2i -Y5i --MAP_FRAME_TYPE=plain >> $ps

