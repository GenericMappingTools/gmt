#!/bin/bash
#
#	$Id$
# Demonstrates +L for horizontal labels for y-axis
ps=horlabel.ps

gmt psbasemap -R-30/30/-20/20 -Baf -Bx+l"Standard horizontal label" -By+l"Long vertical label @~Y(q)@~" -JX5i/3.5i -P -K -Xc > $ps
gmt psbasemap -R -J -Baf -Bx+l"Standard horizontal label" -By+L"@~Y(q)@~" -B+t"Horizontal (+l) and Vertical (+L) Y-axis labels" -O -Y4.75i >> $ps
