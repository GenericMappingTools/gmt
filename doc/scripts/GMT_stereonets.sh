#!/bin/bash
#	$Id$
#
gmt psbasemap -R0/360/-90/90 -JA0/0/1.75i -Bg15 -P -K > GMT_stereonets.ps
echo "180 -90 SCHMIDT" | gmt pstext -R -J -O -K -N -D0/-0.2c -F+f12p,Helvetica-Bold+jTC >> GMT_stereonets.ps
gmt psbasemap -R -JS0/0/1.75i -Bg15 -O -K -X2.75i >> GMT_stereonets.ps
echo "180 -90 WULFF" | gmt pstext -R -J -O -N -D0/-0.2c -F+f12p,Helvetica-Bold+jTC >> GMT_stereonets.ps
