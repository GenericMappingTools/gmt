#!/bin/sh
#	$Id: GMT_stereonets.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

psbasemap -R0/360/-90/90 -JA0/0/1.75i -B0g15 -P -K > GMT_stereonets.ps
echo "180 -90 12 0 1 TC SCHMIDT" | pstext -R -JA -O -K -N -D0/-0.2c >> GMT_stereonets.ps
psbasemap -R -JS0/0/1.75i -B0g15 -O -K -X2.75i >> GMT_stereonets.ps
echo "180 -90 12 0 1 TC WULFF" | pstext -R -JS -O -N -D0/-0.2c >> GMT_stereonets.ps
