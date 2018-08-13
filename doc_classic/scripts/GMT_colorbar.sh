#!/bin/bash
#
ps=GMT_colorbar.ps
gmt makecpt -T-15/15 -Cpolar > t.cpt
gmt psbasemap -R0/20/0/1 -JM5i -BWse -P -K -Baf > $ps
gmt psscale -Ct.cpt -R -J -O -K -Baf -Bx+u"\\232" -By+l@~D@~T -DJBC+e >> $ps
gmt psxy -R -J -O -T >> $ps
