#!/usr/bin/env bash
# Test the two new +c modifier selection:
# a) Not give +c at all: Pick map scale origin as scale bar placement location.
# b) Give +c with no arguments: Pick the middle of the map as map scale origin.

ps=mapbardefault.ps
gmt set FONT_LABEL 9p
gmt psbasemap -R-100/100/0/80 -JM6i -P -Baf -K -Xc > $ps
# Specific latitude [same syntax]
gmt psbasemap -R -J -Lg-50/75+c0+f+w5000k+l"5000 km at Equator"+jTC -O -K >> $ps
gmt psbasemap -R -J -Lg50/75+c0+f+w5000k+l"5000 km at Equator"+jTC -O -K >> $ps
# Get origin at mid-map location [+c no args]
gmt psbasemap -R -J -Lg-50/60+c+f+w5000k+l"5000 km at mid map"+jTC -O -K >> $ps
gmt psbasemap -R -J -Lg50/60+c57.1176+f+w5000k+l"5000 km at mid map"+jTC -O -K >> $ps
# Get origin at scale placement location [no +c]
gmt psbasemap -R -J -Lg-50/30+f+w5000k+l"5000 km at 30N"+jTC -O -K >> $ps
gmt psbasemap -R -J -Lg50/30+c30+f+w5000k+l"5000 km at 30N"+jTC -O >> $ps
