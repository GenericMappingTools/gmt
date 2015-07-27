#!/bin/bash
#
#	$Id: annotations4.sh 13355 2014-07-15 00:25:34Z jluis $
# Demonstrates that the inside annotations gets clipped by boundary; what should we do here?
ps=horlabel.ps

gmt psbasemap -R-30/30/-20/20 -Baf -Bx+l"Standard horizontal label" -By+l"Long vertical label @~Y(q)@~" -JX5i/3.5i -P -K -Xc > $ps
gmt psbasemap -R -J -Baf -Bx+l"Standard horizontal label" -By+L"@~Y(q)@~" -B+t"Horizontal (+l) and Vertical (+L) Y-axis labels" -O -Y4.75i >> $ps
