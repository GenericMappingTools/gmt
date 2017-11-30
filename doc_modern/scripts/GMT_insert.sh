#!/bin/bash
#	$Id$
#
# Testing gmt pslegend capabilities for tables with colors

ps=GMT_insert.ps

# Bottom map of Australia
gmt pscoast -R110E/170E/44S/9S -JM6i -P -Baf -BWSne -Wfaint -N2/1p  -EAU+gbisque -Gbrown -Sazure1 -Da -K -Xc --FORMAT_GEO_MAP=dddF > $ps
gmt psbasemap -R -J -O -K -DjTR+w1.5i+o0.15i+stmp -F+gwhite+p1p+c0.1c+s >> $ps
read x0 y0 w h < tmp
gmt pscoast -Rg -JG120/30S/$w -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque -O -K -X$x0 -Y$y0 >> $ps
gmt psxy -R -J -O -T  -X-${x0} -Y-${y0} >> $ps
