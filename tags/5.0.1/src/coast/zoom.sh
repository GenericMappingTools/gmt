#!/bin/sh
#
#	$Id$
#
#	Use to zoom in on a part of a polygon
#
#
if [ $# -eq 0 ]; then
	echo "usage: zoom.sh w e s n"
	exit
fi
pscoast -R$1/$2/$3/$4 -JM7 -P -B1mg30c -Df -Wthin,blue -K > t.ps
pscoast -R$1/$2/$3/$4 -J -W0.5p -Df -M > t.d
psxy -R -J -O -K t.d -M -Sc0.05i -Gblue >> t.ps
psxy new.d -R -J -O -Wthin,red -M >> t.ps
gv t.ps &
