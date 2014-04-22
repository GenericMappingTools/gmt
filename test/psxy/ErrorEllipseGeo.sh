#!/bin/bash
#	$Id$
# Test clipping of Geographic -SE ellipses across periodic boundaries

ps=ErrorEllipseGeo.ps

gmt psxy -Baf -B+t"Red ellipse should repeat at west boundary" -R-280/80/-70/70 -JM6i -P -K -W3p,blue -Gred -SE -N <<END > $ps
50	0	45	12000	6000
END
gmt psxy -R -J -O -W2p -Ggreen -SE -N <<END >> $ps
50	0	-45	5000	3000
END
