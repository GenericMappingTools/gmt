#!/usr/bin/env bash
# Test clipping of Geographic -SE ellipses across periodic boundaries

ps=ErrorEllipseGeo.ps

gmt psxy -R-280/80/-70/70 -JM6i -P -K -W3p,blue -Gred -SE <<END > $ps
50	0	45	12000	6000
END
gmt psxy -R -J -O -W2p -Ggreen -SE -Baf -B+t"Red ellipse should repeat at west boundary" <<END >> $ps
50	0	-45	5000	3000
END
