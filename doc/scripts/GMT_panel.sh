#!/bin/bash
#	$Id: GMT_anchor.sh 14405 2015-06-26 23:02:31Z pwessel $
#
ps=GMT_panel.ps
gmt psbasemap -R0/2/0/1 -JX5i/2i -B0 -P -K -DjTL+o0.2i+w1.75i/0.75i -F+glightgreen+r > $ps
gmt psbasemap -R -J -O -K -DjBR+o0.3i+w1.75i/0.75i -F+p1p+i+s+gwhite+c0.1i >> $ps
gmt psbasemap -R -J -O -K -DjBR+o0.3i+w1.75i/0.75i -F+p0.25p,-+c0 >> $ps
gmt psxy -R -J -O -T >> $ps
