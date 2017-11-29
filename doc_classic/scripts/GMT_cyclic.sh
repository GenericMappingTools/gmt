#!/bin/bash
#	$Id
#
ps=GMT_cyclic.ps
gmt makecpt -T0/100 -Cjet -Ww > t.cpt
gmt psbasemap -R0/20/0/1 -JM5i -BWse -P -K -Baf > $ps
gmt psscale -Ct.cpt -R -J -O -K -Baf -DJBC >> $ps
gmt psxy -R -J -O -T >> $ps
