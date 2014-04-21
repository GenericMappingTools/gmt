#!/bin/bash
#	$Id$
# Test clipping of Cartesian -Se ellipses across periodic boundaries
ps=ErrorEllipseCart.ps

gmt psxy -Baf -B+t"Red ellipse should repeat at west boundary" -R-280/80/-70/70 -JM6i -P -K -W3p,blue -Gred -Sei <<END > $ps
50	0	30	3	1.5
END
gmt psxy -R -J -O -W2p,white -Ggreen -Sei <<END >> $ps
50	0	120	1	0.5
END
