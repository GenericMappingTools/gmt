#!/bin/sh
#	$Id: GMT_stereographic_polar.sh,v 1.2 2001-09-14 18:30:17 pwessel Exp $
#

gmtset PLOT_DEGREE_FORMAT ddd:mm:ss
pscoast -R-30/30/60/72 -Js0/90/4.5i/60 -Ba10g5/5g5 -Dl -A250 -G0 -P > GMT_stereographic_polar.ps
